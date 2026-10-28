#include "boot_param.h"
#include "bsp_flash.h"
#include <string.h>

/* CRC 覆盖范围：从 magic 起，直到 param_crc32 字段之前的所有字节 */
#define PARAM_CRC_SPAN  (sizeof(boot_param_t) - sizeof(uint32_t))

/**
 * @brief 软件实现的经典 CRC32 (反射式，多项式 0xEDB88320)，与 zlib 结果一致
 */
static uint32_t soft_crc32(const uint8_t *data, uint32_t length)
{
    uint32_t crc = 0xFFFFFFFFU;
    while (length--) {
        crc ^= *data++;
        for (uint8_t i = 0; i < 8; i++) {
            if (crc & 1U) {
                crc = (crc >> 1) ^ 0xEDB88320U;
            } else {
                crc = crc >> 1;
            }
        }
    }
    return ~crc;
}

/**
 * @brief 计算并更新结构体自身的 CRC32
 */
static void boot_param_update_crc(boot_param_t *param)
{
    param->param_crc32 = soft_crc32((const uint8_t *)param, PARAM_CRC_SPAN);
}

/**
 * @brief 校验单份参数是否有效 (魔数 + CRC 双重判断)
 */
static bool boot_param_is_valid(const boot_param_t *param)
{
    if (param->magic != BOOT_PARAM_MAGIC) {
        return false;
    }
    return (soft_crc32((const uint8_t *)param, PARAM_CRC_SPAN) == param->param_crc32);
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

/**
 * @brief 从两个槽中挑选最新的一份
 * @return 最新的有效槽指针；两份都无效时返回 NULL
 */
static const boot_param_t *boot_param_pick_newest(const boot_param_t *s0, bool v0,
                                                  const boot_param_t *s1, bool v1)
{
    if (v0 && v1) {
        return boot_param_seq_is_newer(s1->seq_num, s0->seq_num) ? s1 : s0;
    }
    if (v0) {
        return s0;
    }
    if (v1) {
        return s1;
    }
    return NULL;
}

/**
 * @brief 从 Flash 中读取有效的参数 (双槽比对，取序列号最新的有效副本)
 */
bool boot_param_read(boot_param_t *param)
{
    if (param == NULL) {
        return false;
    }

    const boot_param_t *s0 = (const boot_param_t *)(uintptr_t)PARAM_SLOT_0_ADDR;
    const boot_param_t *s1 = (const boot_param_t *)(uintptr_t)PARAM_SLOT_1_ADDR;

    const boot_param_t *newest = boot_param_pick_newest(s0, boot_param_is_valid(s0),
                                                        s1, boot_param_is_valid(s1));
    if (newest == NULL) {
        return false;   /* 尚未格式化，或两个槽均已损坏 */
    }

    memcpy(param, newest, sizeof(boot_param_t));
    return true;
}

/**
 * @brief 向 Flash 写入参数 (乒乓策略：只擦除较旧的那一份，另一份保持完好)
 *
 * 掉电安全性分析：
 *   - 擦除过程中掉电 -> 被擦的那份失效，另一份仍是完整的旧参数，可正常恢复；
 *   - 写入过程中掉电 -> 目标槽 CRC 不通过，另一份仍有效，可正常恢复；
 *   - 最坏情况只会丢"最近一次写入"的变更，不会退回出厂默认值。
 *
 * 注意：seq_num 在函数内部自增，入参不会被修改。若调用方需要与 Flash 保持一致，
 *       请在写入成功后重新调用 boot_param_read()。
 */
bool boot_param_write(const boot_param_t *param)
{
    if (param == NULL) {
        return false;
    }

    const boot_param_t *s0 = (const boot_param_t *)(uintptr_t)PARAM_SLOT_0_ADDR;
    const boot_param_t *s1 = (const boot_param_t *)(uintptr_t)PARAM_SLOT_1_ADDR;
    bool v0 = boot_param_is_valid(s0);
    bool v1 = boot_param_is_valid(s1);

    uint32_t target_addr;
    uint32_t base_seq;

    if (!v0 && !v1) {
        /* 两槽皆空：格式化到 Slot0 */
        target_addr = PARAM_SLOT_0_ADDR;
        base_seq    = 0U;
    } else if (!v0) {
        /* Slot0 损坏：写 Slot0，顺带完成冗余修复 */
        target_addr = PARAM_SLOT_0_ADDR;
        base_seq    = s1->seq_num;
    } else if (!v1) {
        /* Slot1 损坏：写 Slot1，顺带完成冗余修复 */
        target_addr = PARAM_SLOT_1_ADDR;
        base_seq    = s0->seq_num;
    } else if (boot_param_seq_is_newer(s0->seq_num, s1->seq_num)) {
        /* 两槽都有效：覆盖较旧的那份（这里 s0 更新，故覆盖 s1） */
        target_addr = PARAM_SLOT_1_ADDR;
        base_seq    = s0->seq_num;
    } else {
        target_addr = PARAM_SLOT_0_ADDR;
        base_seq    = s1->seq_num;
    }

    boot_param_t temp;
    memcpy(&temp, param, sizeof(boot_param_t));
    temp.magic   = BOOT_PARAM_MAGIC;
    temp.seq_num = base_seq + 1U;   /* 自然回绕，由 RFC1982 比较规则处理 */
    boot_param_update_crc(&temp);

    /* 仅擦除目标槽所在扇区，另一槽全程保持有效 */
    if (!flash_erase_sector_by_addr(target_addr)) {
        return false;
    }
    if (!flash_write_data(target_addr, (const uint8_t *)&temp, sizeof(boot_param_t))) {
        return false;
    }

    /* 写后回读校验，捕获编程失败 */
    if (!boot_param_is_valid((const boot_param_t *)(uintptr_t)target_addr)) {
        return false;
    }

    return true;
}

/**
 * @brief 参数区初始化检测，若为空则格式化并写入出厂默认值
 */
bool boot_param_init(boot_param_t *param)
{
    if (param == NULL) {
        return false;
    }

    if (boot_param_read(param)) {
        return true;    /* 已有有效参数，直接加载 */
    }

    /* 两个槽都无效，重置为出厂默认参数 */
    memset(param, 0, sizeof(boot_param_t));
    param->magic           = BOOT_PARAM_MAGIC;
    param->seq_num         = 0;                    /* 由 write 内部递增为 1 */
    param->active_slot     = SLOT_APP_A;
    param->upgrade_state   = UPGRADE_STATE_IDLE;
    param->boot_fail_count = 0;
    param->fw_size         = 0;
    param->fw_crc32        = 0;
    param->fw_version      = 0x010000;             /* 默认版本 v1.0.0 */

    if (!boot_param_write(param)) {
        return false;
    }

    /* seq_num 在 write 内部自增，回读一次让调用方拿到真正落盘的内容 */
    return boot_param_read(param);
}
