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
 * 参数区物理布局：Sector 12 / 13 两个独立 16KB 扇区做乒乓备份
 *
 *   Slot 0 : 0x08100000 (Sector 12)  <- 一份完整参数
 *   Slot 1 : 0x08104000 (Sector 13)  <- 另一份完整参数
 *
 * 写入时只擦除"较旧"的那一份所在扇区，另一份全程保持有效。
 * 因此掉电最多损失最后一次写入，不会退回到出厂默认值。
 * Sector 14 ~ 16 暂未使用，留作参数区后续扩展（日志、多版本备份等）。
 * ========================================================================= */
#define PARAM_SLOT_SIZE             0x4000U                             /* 16 KB，物理擦除单位 */
#define PARAM_SLOT_0_ADDR           (PART_PARAM_ADDR)                   /* 0x08100000 */
#define PARAM_SLOT_1_ADDR           (PART_PARAM_ADDR + PARAM_SLOT_SIZE) /* 0x08104000 */

/* 单份参数的有效数据长度上限，留足结构体后续扩展余量 */
#define PARAM_DATA_MAX_SIZE         512U

/* 结构体布局锁定：任何字段变动都会让 Flash 中的旧参数校验失败 */
_Static_assert(sizeof(boot_param_t) == 52U,
               "boot_param_t layout changed, on-flash format would break");
_Static_assert((sizeof(boot_param_t) % 4U) == 0U,
               "boot_param_t size must be a multiple of 4 for word programming");
_Static_assert(sizeof(boot_param_t) <= PARAM_DATA_MAX_SIZE,
               "boot_param_t exceeds PARAM_DATA_MAX_SIZE");
_Static_assert(PARAM_SLOT_SIZE >= PARAM_DATA_MAX_SIZE,
               "parameter slot smaller than one parameter block");
/* 两个槽必须完整落在 PART_PARAM 分区内，防止调整槽大小或增加备份槽时越界 */
_Static_assert((PARAM_SLOT_1_ADDR + PARAM_SLOT_SIZE) <= (PART_PARAM_ADDR + PART_PARAM_SIZE),
               "parameter slots exceed the PART_PARAM area");


/* 函数声明 */
bool boot_param_selfcheck(void);
bool boot_param_init(boot_param_t *param);
bool boot_param_read(boot_param_t *param);
bool boot_param_write(const boot_param_t *param);

#endif /* BOOT_PARAM_H */
