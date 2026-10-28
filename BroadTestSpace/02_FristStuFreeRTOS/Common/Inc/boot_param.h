#ifndef BOOT_PARAM_H
#define BOOT_PARAM_H

#include <stdint.h>
#include <stdbool.h>
#include "boot_partition.h"

/* 识别魔数 (ASCII "BOOT" 的小端表示)，表示参数区已经被有效初始化 */
#define BOOT_PARAM_MAGIC            0x544F4F42U

/* 连续启动失败的最大允许阈值，达到后由 Bootloader 触发自动回滚 */
#define BOOT_MAX_FAIL_COUNT         3U

/* 固件运行所在分区枚举 */
typedef enum {
    SLOT_APP_A = 0,
    SLOT_APP_B = 1,
} boot_slot_enum;

/* OTA 升级状态机枚举 */
typedef enum {
    UPGRADE_STATE_IDLE = 0,      /* 正常空闲状态，无需升级 */
    UPGRADE_STATE_NEED_UPGRADE,  /* 收到升级包，等待 Bootloader 搬运或校验 */
    UPGRADE_STATE_TESTING,       /* 新固件首次启动测试中，等待 App 确认健康 */
    UPGRADE_STATE_ROLLBACK       /* 固件异常，需要执行回滚 */
} upgrade_state_enum;

/*
 * 参数结构体：固定 52 字节，自然 4 字节对齐，无填充空洞
 * 必须与 Flash 中已经存在的旧数据保持二进制兼容，改动需同步版本升级策略
 */
typedef struct {
    uint32_t magic;              /* 魔数: 0x544F4F42 */
    uint32_t seq_num;            /* 递增序列号，用于在双备份中挑选最新一份 */

    uint8_t  active_slot;        /* 当前激活执行的分区 (boot_slot_enum) */
    uint8_t  upgrade_state;      /* 当前升级流程状态 (upgrade_state_enum) */
    uint8_t  boot_fail_count;    /* 连续启动失败计数器 */
    uint8_t  reserved1;          /* 预留字节，补齐 4 字节对齐 */

    uint32_t fw_size;            /* 待校验/激活槽内固件的实际字节大小 */
    uint32_t fw_crc32;           /* 待校验/激活槽内固件镜像的整体 CRC32 */
    uint32_t fw_version;         /* 固件版本号，例如 0x010002 表示 v1.0.2 */

    uint32_t reserved[6];        /* 预留字段，为日后扩展保留空间 */

    uint32_t param_crc32;        /* 本结构体自身的 CRC32 (从 magic 算到 reserved 尾部) */
} boot_param_t;


/* =========================================================================
 * 参数区物理布局：两个 16KB 扇区交替作为「追加式日志」，S14~S16 留作扩展
 *
 *   Region A : 0x08100000 (Sector 12)
 *   Region B : 0x08104000 (Sector 13)
 *
 * 写入采用追加（append-only）：每次写占用一个空槽，不擦除任何旧槽。
 * 当前区写满后，切换到另一区并从槽 0 重新开始，切换前先擦除目标区——
 * 此时目标区必然不含最新数据，因此擦除期间另一区始终完整。
 *
 * 相比"每次写都擦扇区"的乒乓备份：
 *   - 掉电安全：任意时刻至少有一个区保存着完整的最新参数；
 *   - 擦除次数从"每次写一次"降到"每 128 次写一次"，显著延长扇区寿命。
 * ========================================================================= */
#define PARAM_REGION_SIZE           0x4000U                             /* 16 KB，单区 = 一个物理扇区 */
#define PARAM_REGION_A_ADDR         (PART_PARAM_ADDR)                   /* 0x08100000 */
#define PARAM_REGION_B_ADDR         (PART_PARAM_ADDR + PARAM_REGION_SIZE) /* 0x08104000 */

/* 单个数据槽大小：boot_param_t 目前 52 字节，留出扩展余量 */
#define PARAM_SLOT_SIZE             128U
#define PARAM_SLOTS_PER_REGION      (PARAM_REGION_SIZE / PARAM_SLOT_SIZE) /* 128 */

/* 结构体布局锁定：任何字段变动都会让 Flash 中的旧参数校验失败 */
_Static_assert(sizeof(boot_param_t) == 52U,
               "boot_param_t layout changed, on-flash format would break");
_Static_assert((sizeof(boot_param_t) % 4U) == 0U,
               "boot_param_t size must be a multiple of 4 for word programming");
_Static_assert(sizeof(boot_param_t) <= PARAM_SLOT_SIZE,
               "boot_param_t exceeds one log slot");
_Static_assert((PARAM_SLOT_SIZE % 4U) == 0U,
               "slot size must be 4-byte aligned for word programming");
_Static_assert((PARAM_REGION_SIZE % PARAM_SLOT_SIZE) == 0U,
               "region size must be an integer multiple of slot size");
/* 两个日志区必须完整落在 PART_PARAM 分区内 */
_Static_assert((PARAM_REGION_B_ADDR + PARAM_REGION_SIZE) <= (PART_PARAM_ADDR + PART_PARAM_SIZE),
               "parameter log regions exceed the PART_PARAM area");


/* 函数声明 */
bool boot_param_selfcheck(void);
bool boot_param_init(boot_param_t *param);
bool boot_param_read(boot_param_t *param);
bool boot_param_write(const boot_param_t *param);

#endif /* BOOT_PARAM_H */
