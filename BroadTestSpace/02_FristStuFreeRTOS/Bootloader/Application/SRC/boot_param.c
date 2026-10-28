#include "boot_param.h"
#include "boot_crc32.h"
#include "bsp_flash.h"
#include <string.h>

/* CRC 覆盖范围：从 magic 起，直到 param_crc32 字段之前的所有字节 */
#define PARAM_CRC_SPAN  (sizeof(boot_param_t) - sizeof(uint32_t))

/**
 * @brief 计算并更新结构体自身的 CRC32
 */
static void boot_param_update_crc(boot_param_t *param)
{
    param->param_crc32 = boot_crc32_calc((const uint8_t *)param, PARAM_CRC_SPAN);
}

/**
 * @brief 校验单份参数是否有效 (魔数 + CRC 双重判断)
 */
static bool boot_param_is_valid(const boot_param_t *param)
{
    if (param->magic != BOOT_PARAM_MAGIC) {
        return false;
    }
    return (boot_crc32_calc((const uint8_t *)param, PARAM_CRC_SPAN) == param->param_crc32);
}

/**
 * @brief 序列号比较：按 RFC1982 序列号算术判断 a 是否比 b 更新
 * @note  直接用 uint32 大小比较会在 0xFFFFFFFF -> 0 回绕时选错副本，
 *        这里用有符号差值规避（二补码平台上的标准做法）
 */
static bool boot_param_seq_is_newer(uint32_t a, uint32_t b)
{
    return ((int32_t)(a - b) > 0);
}

/* 单个日志区的扫描结果 */
typedef struct {
    int32_t  newest_slot;   /* 本区最新有效槽索引，-1 表示本区无有效数据 */
    uint32_t newest_seq;    /* 对应的 seq_num */
    int32_t  free_slot;     /* 第一个空槽索引，-1 表示本区已写满 */
} param_region_info_t;

static uint32_t param_region_base(uint32_t region)
{
    return (region == 0U) ? PARAM_REGION_A_ADDR : PARAM_REGION_B_ADDR;
}

static const boot_param_t *param_slot_ptr(uint32_t region, uint32_t slot)
{
    uint32_t addr = param_region_base(region) + (slot * PARAM_SLOT_SIZE);
    return (const boot_param_t *)(uintptr_t)addr;
}

/**
 * @brief 扫描单个日志区
 * @note  追加式日志保证：遇到第一个空槽（首字为全 0xFF）之后不会再有有效数据，
 *        因此可以提前结束扫描，避免每次启动都把 128 个槽全读一遍
 */
static void param_region_scan(uint32_t region, param_region_info_t *info)
{
    info->newest_slot = -1;
    info->newest_seq  = 0U;
    info->free_slot   = -1;

    for (uint32_t i = 0U; i < PARAM_SLOTS_PER_REGION; i++) {
        const boot_param_t *p = param_slot_ptr(region, i);

        if (p->magic == 0xFFFFFFFFU) {
            info->free_slot = (int32_t)i;
            break;
        }
        if (boot_param_is_valid(p)) {
            if ((info->newest_slot < 0) ||
                boot_param_seq_is_newer(p->seq_num, info->newest_seq)) {
                info->newest_slot = (int32_t)i;
                info->newest_seq  = p->seq_num;
            }
        }
    }
}

/**
 * @brief 从两个日志区中读取最新的一份有效参数
 */
bool boot_param_read(boot_param_t *param)
{
    if (param == NULL) {
        return false;
    }

    param_region_info_t info[2];
    param_region_scan(0U, &info[0]);
    param_region_scan(1U, &info[1]);

    int32_t  best_region = -1;
    uint32_t best_seq    = 0U;

    if (info[0].newest_slot >= 0) {
        best_region = 0;
        best_seq    = info[0].newest_seq;
    }
    if ((info[1].newest_slot >= 0) &&
        ((best_region < 0) || boot_param_seq_is_newer(info[1].newest_seq, best_seq))) {
        best_region = 1;
        best_seq    = info[1].newest_seq;
    }

    if (best_region < 0) {
        return false;   /* 两个区都还没有有效数据 */
    }

    memcpy(param,
           param_slot_ptr((uint32_t)best_region, (uint32_t)info[best_region].newest_slot),
           sizeof(boot_param_t));
    return true;
}

/**
 * @brief 追加写入一份新参数
 *
 * 策略：
 *   1. 优先在当前活动区（含最新参数的那个区）里追加到第一个空槽，不擦除任何旧槽；
 *   2. 活动区写满后切换另一区并从槽 0 开始，切换前擦除该区——此时它必然不含
 *      最新数据，所以擦除期间活动区始终完整可读；
 *   3. 两个区都空时，从 A 区槽 0 开始。
 *
 * 掉电安全性：任意时刻至少有一个区保存着完整的最新参数，最坏只丢最后一次写入。
 * 磨损：每 PARAM_SLOTS_PER_REGION 次写入才擦除一次扇区，而非每次写入都擦。
 *
 * 注意：seq_num 在函数内部基于 Flash 中的最新值自增，入参不会被修改。
 *       若调用方需要与 Flash 保持一致，请在写入成功后重新调用 boot_param_read()。
 */
bool boot_param_write(const boot_param_t *param)
{
    if (param == NULL) {
        return false;
    }

    param_region_info_t info[2];
    param_region_scan(0U, &info[0]);
    param_region_scan(1U, &info[1]);

    int32_t  active   = -1;     /* 含最新参数的那个区 */
    uint32_t base_seq = 0U;

    if (info[0].newest_slot >= 0) {
        active   = 0;
        base_seq = info[0].newest_seq;
    }
    if ((info[1].newest_slot >= 0) &&
        ((active < 0) || boot_param_seq_is_newer(info[1].newest_seq, base_seq))) {
        active   = 1;
        base_seq = info[1].newest_seq;
    }

    uint32_t target_region;
    uint32_t target_slot;

    if (active < 0) {
        /* 两区皆空：从 A 区槽 0 开始 */
        target_region = 0U;
        target_slot   = 0U;
        base_seq      = 0U;
    } else if (info[active].free_slot >= 0) {
        /* 活动区还有空槽：直接追加，不擦除 */
        target_region = (uint32_t)active;
        target_slot   = (uint32_t)info[active].free_slot;
    } else {
        /* 活动区写满：切到另一区，先擦除（该区不含最新数据，擦除安全） */
        target_region = (uint32_t)(1 - active);
        if (!flash_erase_sector_by_addr(param_region_base(target_region))) {
            return false;
        }
        target_slot = 0U;
    }

    boot_param_t temp;
    memcpy(&temp, param, sizeof(boot_param_t));
    temp.magic   = BOOT_PARAM_MAGIC;
    temp.seq_num = base_seq + 1U;   /* 自然回绕，由 RFC1982 比较规则处理 */
    boot_param_update_crc(&temp);

    uint32_t addr = param_region_base(target_region) + (target_slot * PARAM_SLOT_SIZE);
    if (!flash_write_data(addr, (const uint8_t *)&temp, sizeof(boot_param_t))) {
        return false;
    }

    /* 写后回读校验，捕获编程失败 */
    if (!boot_param_is_valid((const boot_param_t *)(uintptr_t)addr)) {
        return false;
    }

    return true;
}

/**
 * @brief 分区表与 BSP 扇区表的交叉自检
 *
 * boot_param.h 里的槽地址是手工维护的常量，而 bsp_flash.c 的扇区表是另一份
 * 完全独立的定义。两处只要有一处改漏，擦写就会落到错误的物理扇区上，而且
 * 编译器不会报任何错。这里在启动时做一次交叉核对，把这种静默漂移变成明确的失败。
 *
 * @return true 表示槽地址与 BSP 扇区划分一致
 */
bool boot_param_selfcheck(void)
{
    /* 每个日志区必须正好覆盖一个完整的 16KB 扇区（地址越界时 BSP 返回 0） */
    if (flash_sector_size_get(PARAM_REGION_A_ADDR) != PARAM_REGION_SIZE) {
        return false;
    }
    if (flash_sector_size_get(PARAM_REGION_B_ADDR) != PARAM_REGION_SIZE) {
        return false;
    }

    /* 两区必须落在不同扇区，否则切换区时会擦掉自己刚写完的日志 */
    return (PARAM_REGION_B_ADDR != PARAM_REGION_A_ADDR);
}

/**
 * @brief 参数区初始化检测，若为空则格式化并写入出厂默认值
 */
bool boot_param_init(boot_param_t *param)
{
    if (param == NULL) {
        return false;
    }

    /* 分区表与 BSP 扇区表不一致时直接拒绝服务：继续读写会擦错扇区 */
    if (!boot_param_selfcheck()) {
        return false;
    }

    if (boot_param_read(param)) {
        return true;    /* 已有有效参数，直接加载 */
    }

    /* 两个日志区都没有有效数据，重置为出厂默认参数 */
    memset(param, 0, sizeof(boot_param_t));
    param->magic           = BOOT_PARAM_MAGIC;
    param->seq_num         = 0;                    /* 由 write 内部递增为 1 */
    param->active_slot     = SLOT_APP_A;
    param->upgrade_state   = UPGRADE_STATE_IDLE;
    param->boot_fail_count = 0;
    /* memset 已清零，只需给出 A 槽的默认版本；B 槽出厂为空 */
    PARAM_SLOT_INFO(param, SLOT_APP_A).fw_version = 0x010000;   /* v1.0.0 */

    if (!boot_param_write(param)) {
        return false;
    }

    /* seq_num 在 write 内部自增，回读一次让调用方拿到真正落盘的内容 */
    return boot_param_read(param);
}
