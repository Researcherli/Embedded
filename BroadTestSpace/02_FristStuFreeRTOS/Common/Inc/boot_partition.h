#ifndef BOOT_PARTITION_H
#define BOOT_PARTITION_H

#include <stdint.h>

/* =========================================================================
 * 1. 片上 2MB Flash 整体范围定义 (GD32F4xx, 双 Bank 各 1MB)
 * ========================================================================= */
#define FLASH_START_ADDR        0x08000000U
#define FLASH_TOTAL_SIZE        (2048U * 1024U)                         /* 2 MB */
#define FLASH_END_ADDR          (FLASH_START_ADDR + FLASH_TOTAL_SIZE)   /* 0x08200000，排他端 */

/* 双 Bank 边界 */
#define FLASH_BANK0_START_ADDR  0x08000000U
#define FLASH_BANK1_START_ADDR  0x08100000U


/* =========================================================================
 * 2. 逻辑分区布局
 *
 *    Bank0 (1 MB)                        Bank1 (1 MB)
 *    +---------------------------+       +---------------------------+
 *    | Boot    128 KB  S0 ~ S4   |       | Param   128 KB  S12 ~ S16 |
 *    | App-A   768 KB  S5 ~ S10  |       | App-B   768 KB  S17 ~ S22 |
 *    | User    128 KB  S11       |       | Rsvd    128 KB  S23       |
 *    +---------------------------+       +---------------------------+
 *
 * 设计要点：
 *   1. 参数区放在 Bank1，与 Bootloader 不在同一个 Bank —— 擦写参数时
 *      Bank0 的取指不会被阻塞，Bootloader 不会因此长时间停摆；
 *   2. App-A / App-B 容量必须严格相等，否则 OTA 镜像无法在两个槽间互换；
 *   3. 所有分区起点与长度必须是 16KB 扇区整数倍（见文件末尾静态断言）。
 * ========================================================================= */

/* [分区 1] Bootloader 固件区 (Sector 0 ~ 4: 4×16KB + 64KB = 128KB) */
#define PART_BOOT_ADDR          0x08000000U
#define PART_BOOT_SIZE          (128U * 1024U)                          /* 128 KB */

/* [分区 2] App-A 主运行区 (Sector 5 ~ 10: 6×128KB) */
#define PART_APP_A_ADDR         0x08020000U
#define PART_APP_A_SIZE         (768U * 1024U)                          /* 768 KB */

/* [分区 3] 用户数据区 (Sector 11)，预留给 LittleFS / 出厂参数 */
#define PART_USER_DATA_ADDR     0x080E0000U
#define PART_USER_DATA_SIZE     (128U * 1024U)                          /* 128 KB */

/* [分区 4] Bootloader 参数区 (Sector 12 ~ 16)
 * 内部再拆成两个独立的 16KB 扇区做乒乓备份，详见 boot_param.h */
#define PART_PARAM_ADDR         0x08100000U
#define PART_PARAM_SIZE         (128U * 1024U)                          /* 128 KB */

/* [分区 5] App-B 下载备份区 (Sector 17 ~ 22: 6×128KB) */
#define PART_APP_B_ADDR         0x08120000U
#define PART_APP_B_SIZE         (768U * 1024U)                          /* 768 KB */

/* [分区 6] 预留区 (Sector 23)，暂不分配，留给后续功能扩展 */
#define PART_RESERVED_ADDR      0x081E0000U
#define PART_RESERVED_SIZE      (128U * 1024U)                          /* 128 KB */


/* App 向量表相对 Flash 基址的偏移，供 nvic_vector_table_set() 使用 */
#define PART_APP_A_VTOR_OFFSET  (PART_APP_A_ADDR - FLASH_START_ADDR)    /* 0x00020000 */
#define PART_APP_B_VTOR_OFFSET  (PART_APP_B_ADDR - FLASH_START_ADDR)    /* 0x00120000 */


/* =========================================================================
 * 3. 分区合法性静态校验 (编译期检查，防止粗心改错地址)
 *    用法：在任意 .c 文件作用域写  STATIC_ASSERT_PARTITION();
 * ========================================================================= */
#define STATIC_ASSERT_PARTITION() \
    /* 相邻分区首尾相接：既无空洞也无重叠 */ \
    _Static_assert((PART_BOOT_ADDR + PART_BOOT_SIZE) == PART_APP_A_ADDR,           "Boot & App-A overlap/gap"); \
    _Static_assert((PART_APP_A_ADDR + PART_APP_A_SIZE) == PART_USER_DATA_ADDR,     "App-A & UserData overlap/gap"); \
    _Static_assert((PART_USER_DATA_ADDR + PART_USER_DATA_SIZE) == PART_PARAM_ADDR, "UserData & Param overlap/gap"); \
    _Static_assert((PART_PARAM_ADDR + PART_PARAM_SIZE) == PART_APP_B_ADDR,         "Param & App-B overlap/gap"); \
    _Static_assert((PART_APP_B_ADDR + PART_APP_B_SIZE) == PART_RESERVED_ADDR,      "App-B & Reserved overlap/gap"); \
    _Static_assert((PART_RESERVED_ADDR + PART_RESERVED_SIZE) == FLASH_END_ADDR,    "Flash total size mismatch"); \
    /* OTA 前提：两个 App 槽必须等大 */ \
    _Static_assert(PART_APP_A_SIZE == PART_APP_B_SIZE,                             "App-A and App-B must be equal size"); \
    /* 所有分区起点必须落在 16KB 扇区边界上 */ \
    _Static_assert((((PART_BOOT_ADDR | PART_APP_A_ADDR | PART_USER_DATA_ADDR | \
                      PART_PARAM_ADDR | PART_APP_B_ADDR | PART_RESERVED_ADDR) & 0x3FFFU) == 0U), \
                   "partition start address is not 16KB aligned"); \
    /* 所有分区长度必须是 16KB 整数倍，否则跨扇区 */ \
    _Static_assert((((PART_BOOT_SIZE | PART_APP_A_SIZE | PART_USER_DATA_SIZE | \
                      PART_PARAM_SIZE | PART_APP_B_SIZE | PART_RESERVED_SIZE) & 0x3FFFU) == 0U), \
                   "partition size is not 16KB aligned")


/* =========================================================================
 * 4. 辅助检查宏函数 (供代码运行时做防御性安全拦截)
 * ========================================================================= */

/**
 * @brief 判断地址是否落在指定分区内
 */
#define IS_IN_PARTITION(addr, part_addr, part_size) \
    (((uint32_t)(addr) >= (part_addr)) && ((uint32_t)(addr) < ((part_addr) + (part_size))))

/**
 * @brief 专供 App-A 和 App-B 的范围有效性判定
 */
#define IS_ADDR_IN_APP_A(addr)  IS_IN_PARTITION((addr), PART_APP_A_ADDR, PART_APP_A_SIZE)
#define IS_ADDR_IN_APP_B(addr)  IS_IN_PARTITION((addr), PART_APP_B_ADDR, PART_APP_B_SIZE)

#endif /* BOOT_PARTITION_H */
