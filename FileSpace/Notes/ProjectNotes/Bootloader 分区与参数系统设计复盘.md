# GD32F450 Bootloader 分区与参数系统设计复盘

> 对象：`BroadTestSpace/02_FristStuFreeRTOS`（Bootloader 侧）
> 范围：从三个空壳文件，到分区定稿、参数系统跑通、镜像校验接入
> 时间：2026-10-28
> 阅读建议：第 1 章先建立文件地图，第 2–7 章逐文件读代码，第 8 章看「为什么演变成现在这样」，第 11 章是可以直接查的坑表

---

## 0. 这份文档怎么读

每个设计点都按同一个套路写：

```
【现象/问题】→【为什么会有这个问题】→【代码怎么写的】→【不这么写会怎样】
```

代码块左上角标了文件路径，`L12` 这种标的是行号，方便你直接跳到 Keil 里对照。

先记住三句话，后面所有细节都挂在它们上面：

1. **Flash 只能整扇区擦，只能把 1 写成 0** —— 这条物理约束决定了参数区的一切设计
2. **擦除单位必须等于逻辑单元** —— 否则会出现回收不了的空间
3. **能在编译期抓的错误，绝不放到运行时** —— 这是断言体系的意义

---

## 1. 文件地图

### 1.1 参与 Bootloader 的文件

```
02_FristStuFreeRTOS/
├── Common/
│   ├── Inc/
│   │   ├── boot_partition.h     分区表：地址、大小、编译期断言
│   │   ├── boot_param.h         参数结构体、日志区布局、函数声明
│   │   ├── boot_crc32.h         CRC32 接口
│   │   └── boot_image.h         镜像校验接口
│   └── Src/
│       ├── boot_crc32.c         CRC32 查表实现
│       └── boot_image.c         镜像 CRC 校验实现
├── Bootloader/
│   ├── Application/SRC/
│   │   ├── main.c               启动流程：参数加载 → 校验 → 回滚判断
│   │   ├── boot_param.c         参数读写：双区追加日志
│   │   ├── systick.c            延时
│   │   └── gd32f4xx_it.c        中断向量实现
│   └── BSP/
│       └── bsp_flash.c          物理扇区表 + 擦写函数
└── Application/                  App 工程（共享 Common/Inc）
```

### 1.2 调用关系

```
main.c
 ├─ boot_param_init() ──► boot_param_selfcheck()   交叉校验扇区表
 │                    └─ boot_param_read()         扫描两区
 │                    └─ boot_param_write()        追加写入
 │                         └─ flash_erase_sector_by_addr()   BSP
 │                         └─ flash_write_data()             BSP
 │                         └─ boot_param_update_crc() ──► boot_crc32_calc()
 │
 ├─ boot_image_verify() ──► boot_crc32_update()    分块累加
 │                     └─ flash 直接指针读取
 │
 └─ boot_handle_image_failure() ──► boot_param_write()
                                └─ NVIC_SystemReset()
```

### 1.3 每一层为什么要分开

| 模块 | 为什么单独存在 |
|---|---|
| `boot_partition.h` | 地址是唯一真相。App 和 Bootloader 都要引，放公共目录避免两边各写一份 |
| `boot_crc32.c` | 参数校验和镜像校验都要算 CRC。如果各写一份，算法漂移了都不知道 |
| `boot_image.c` | 校验逻辑与参数存储无关，将来 App 侧「确认镜像健康」也要用 |
| `boot_param.c` | 只管「怎么把结构体安全地存进 Flash」，不关心业务语义 |
| `main.c` | 只管「启动顺序」，决策逻辑集中在一处，方便改 |

---

## 2. `boot_partition.h` —— 分区表

### 2.1 为什么整个工程只有这一份地址定义

`Common/Inc/boot_partition.h` L9-15：

```c
#define FLASH_START_ADDR        0x08000000U
#define FLASH_TOTAL_SIZE        (2048U * 1024U)                         /* 2 MB */
#define FLASH_END_ADDR          (FLASH_START_ADDR + FLASH_TOTAL_SIZE)   /* 0x08200000，排他端 */

/* 双 Bank 边界 */
#define FLASH_BANK0_START_ADDR  0x08000000U
#define FLASH_BANK1_START_ADDR  0x08100000U
```

**为什么 `FLASH_END_ADDR` 是 0x08200000 而不是 0x081FFFFF**：这里用的是「排他端」语义，即「最后一个字节的下一个地址」。这样判断区间用 `addr < END` 就够了，不用写 `<= 0x081FFFFF`，少一个容易写错的常量。

后面的静态断言 `PART_RESERVED_ADDR + PART_RESERVED_SIZE == FLASH_END_ADDR` 正是依赖这个语义才能成立。

**为什么带 `U` 后缀**：不用 `U` 的话，`2048 * 1024` 在某些编译器上会被当成 `int` 运算，接近边界时可能出问题。全部标 `U` 是嵌入式里的习惯做法。

### 2.2 六个分区，每个地址是怎么算出来的

`boot_partition.h` L36-58：

```c
/* [分区 1] Bootloader 固件区 (Sector 0 ~ 4: 4×16KB + 64KB = 128KB) */
#define PART_BOOT_ADDR          0x08000000U
#define PART_BOOT_SIZE          (128U * 1024U)                          /* 128 KB */

/* [分区 2] App-A 主运行区 (Sector 5 ~ 10: 6×128KB) */
#define PART_APP_A_ADDR         0x08020000U
#define PART_APP_A_SIZE         (768U * 1024U)                          /* 768 KB */

/* [分区 3] 用户数据区 (Sector 11)，预留给 LittleFS / 出厂参数 */
#define PART_USER_DATA_ADDR     0x080E0000U
#define PART_USER_DATA_SIZE     (128U * 1024U)                          /* 128 KB */

/* [分区 4] Bootloader 参数区 (Sector 12 ~ 16) */
#define PART_PARAM_ADDR         0x08100000U
#define PART_PARAM_SIZE         (128U * 1024U)                          /* 128 KB */

/* [分区 5] App-B 下载备份区 (Sector 17 ~ 22: 6×128KB) */
#define PART_APP_B_ADDR         0x08120000U
#define PART_APP_B_SIZE         (768U * 1024U)                          /* 768 KB */

/* [分区 6] 预留区 (Sector 23)，暂不分配，留给后续功能扩展 */
#define PART_RESERVED_ADDR      0x081E0000U
#define PART_RESERVED_SIZE      (128U * 1024U)                          /* 128 KB */
```

**为什么要按扇区边界对齐**：GD32F4 的扇区不是等大的。Bank0 是 `4×16KB + 1×64KB + 7×128KB`，Bank1 也是同样结构：

| 扇区 | 起始 | 大小 |
|---|---|---|
| S0–S3 | 0x08000000 | 各 16 KB |
| S4 | 0x08010000 | 64 KB |
| S5–S11 | 0x08020000 | 各 128 KB |
| S12–S15 | 0x08100000 | 各 16 KB |
| S16 | 0x08110000 | 64 KB |
| S17–S23 | 0x08120000 | 各 128 KB |

如果一个分区的起点落在某个扇区中间，擦除这个分区时会把**相邻分区的一部分也擦掉**。这是不可恢复的。所以每个分区起点都必须正好是某个扇区的起点。

**为什么参数区放在 Bank1（0x08100000）**，这是整套设计里最关键的一个决定，理由在第 3 章详述。

### 2.3 VTOR 偏移为什么用宏算，不写死

`boot_partition.h` L62-63：

```c
#define PART_APP_A_VTOR_OFFSET  (PART_APP_A_ADDR - FLASH_START_ADDR)    /* 0x00020000 */
#define PART_APP_B_VTOR_OFFSET  (PART_APP_B_ADDR - FLASH_START_ADDR)    /* 0x00120000 */
```

**历史教训**：改造前 App 的 `main.c` 里写的是 `nvic_vector_table_set(NVIC_VECTTAB_FLASH, 0x40000)`，而 scatter 文件写的是 `0x08010000`，分区表写的是 `0x08040000`。

三处地址全不一样：编译能过，下载能跑，但一经过 Bootloader 跳转就必崩（向量表指向了错误的物理地址），而且崩得莫名其妙。

改成由宏推算后，改分区表地址，VTOR 自动跟着变，物理上不可能再对不上。

### 2.4 九条静态断言，每一条防什么

`boot_partition.h` L70-87：

```c
#define STATIC_ASSERT_PARTITION() \
    _Static_assert((PART_BOOT_ADDR + PART_BOOT_SIZE) == PART_APP_A_ADDR,           "Boot & App-A overlap/gap"); \
    _Static_assert((PART_APP_A_ADDR + PART_APP_A_SIZE) == PART_USER_DATA_ADDR,     "App-A & UserData overlap/gap"); \
    _Static_assert((PART_USER_DATA_ADDR + PART_USER_DATA_SIZE) == PART_PARAM_ADDR, "UserData & Param overlap/gap"); \
    _Static_assert((PART_PARAM_ADDR + PART_PARAM_SIZE) == PART_APP_B_ADDR,         "Param & App-B overlap/gap"); \
    _Static_assert((PART_APP_B_ADDR + PART_APP_B_SIZE) == PART_RESERVED_ADDR,      "App-B & Reserved overlap/gap"); \
    _Static_assert((PART_RESERVED_ADDR + PART_RESERVED_SIZE) == FLASH_END_ADDR,    "Flash total size mismatch"); \
    _Static_assert(PART_APP_A_SIZE == PART_APP_B_SIZE,                             "App-A and App-B must be equal size"); \
    _Static_assert((((PART_BOOT_ADDR | PART_APP_A_ADDR | PART_USER_DATA_ADDR | \
                      PART_PARAM_ADDR | PART_APP_B_ADDR | PART_RESERVED_ADDR) & 0x3FFFU) == 0U), \
                   "partition start address is not 16KB aligned"); \
    _Static_assert((((PART_BOOT_SIZE | PART_APP_A_SIZE | PART_USER_DATA_SIZE | \
                      PART_PARAM_SIZE | PART_APP_B_SIZE | PART_RESERVED_SIZE) & 0x3FFFU) == 0U), \
                   "partition size is not 16KB aligned")
```

逐条解释：

| 行 | 检查 | 不检查会怎样 |
|---|---|---|
| L72-77 | 六段首尾相接 | 改地址时漏改一处，产生空洞（地址不可用）或重叠（两个分区抢同一块 Flash）|
| L79 | A/B 等大 | OTA 时 B 槽放不下和 A 槽一样大的镜像，升级直接失败 |
| L81-83 | 起点 16KB 对齐 | 分区起点落在扇区中间，擦除误伤邻区 |
| L85-87 | 长度 16KB 整数倍 | 分区尾部跨扇区，擦除时把下一分区的数据带走 |

**为什么把六个常量用 `|` 串起来一次判断**：

```c
(PART_BOOT_ADDR | PART_APP_A_ADDR | ... | PART_RESERVED_ADDR) & 0x3FFFU) == 0U
```

`0x3FFF` 是 16KB 对齐的掩码（16KB = 0x4000，减一是 0x3FFF）。六个数的低 14 位只要有一个不为 0，按位或之后就不为 0。所以这一条等价于「六个都对齐」，但只占三行，比写六条断言短得多，也更容易看出意图。

**为什么用 `_Static_assert` 而不是 `assert`**：
- `_Static_assert` 在**编译期**求值，不生成任何指令，不占 ROM 不占 RAM
- `assert` 是运行时检查，占代码空间，而且在 Bootloader 里崩溃了也没人能告诉你

代价是：表达式必须是编译期常量。这里全是宏，满足。

### 2.5 为什么每个工程都要单独调用一次

`boot_partition.h` L68 注释写着「在任意 .c 文件作用域写 STATIC_ASSERT_PARTITION();」，实际调用点是：

- `Bootloader/Application/SRC/main.c` L10：`STATIC_ASSERT_PARTITION();`
- `Application/Application/SRC/main.c`：同样一行

**关键**：宏定义在头文件里，但只有**被展开（调用）**时才真正产生断言。如果 App 工程只 `#include "boot_partition.h"` 而不调用这个宏，那么分区表被改坏时：

- Bootloader 编译会报错 ✓
- App 编译**不会报错**，但会算出错误的 VTOR ✗

这就是为什么两个 main.c 都要写一行。

### 2.6 运行时用的区间判断宏

`boot_partition.h` L97-104：

```c
#define IS_IN_PARTITION(addr, part_addr, part_size) \
    (((uint32_t)(addr) >= (part_addr)) && ((uint32_t)(addr) < ((part_addr) + (part_size))))

#define IS_ADDR_IN_APP_A(addr)  IS_IN_PARTITION((addr), PART_APP_A_ADDR, PART_APP_A_SIZE)
#define IS_ADDR_IN_APP_B(addr)  IS_IN_PARTITION((addr), PART_APP_B_ADDR, PART_APP_B_SIZE)
```

**注意右边是 `<` 不是 `<=`**：因为 `part_addr + part_size` 是「下一个分区」的起点，不包含在本区间内。用 `<=` 会把下一个分区的第一个字节误判成本分区的，在边界检查里这是致命的。

**为什么每个参数都套括号**：宏是文本替换。如果调用 `IS_IN_PARTITION(a + 1, ...)`，不套括号展开成 `a + 1 >= 0x08000000` 没问题，但遇到 `&`、`|` 这类低优先级运算符就会出错。宏参数一律套括号是铁律。

---

## 3. `boot_param.h` —— 参数结构与日志布局

### 3.1 魔数

`boot_param.h` L10：

```c
#define BOOT_PARAM_MAGIC            0x544F4F42U
```

`0x544F4F42` 在**小端内存**里的字节序是 `42 4F 4F 54`，即 ASCII 的 `B O O T`。

**为什么用可读字符串而不是随便一个数**：调试时用 J-Link 直接看 Flash，一眼就能认出「这里是参数区」。如果是个随机数，你每次都得多查一次文档。

**为什么需要魔数**（有 CRC 不就行了吗）：不是所有 Flash 位置都有参数。全新芯片是 `0xFF`，擦除后也是 `0xFF`。先判断魔数可以**在算 CRC 之前**快速排除空槽：

```c
if (param->magic != BOOT_PARAM_MAGIC) {
    return false;    // 立刻返回，省掉 60 字节的 CRC 计算
}
```

扫描 256 个槽时，这个提前返回能省掉大量无用计算。

### 3.2 两个枚举

`boot_param.h` L16-27：

```c
typedef enum {
    SLOT_APP_A = 0,
    SLOT_APP_B = 1,
} boot_slot_enum;

typedef enum {
    UPGRADE_STATE_IDLE = 0,
    UPGRADE_STATE_NEED_UPGRADE,
    UPGRADE_STATE_TESTING,
    UPGRADE_STATE_ROLLBACK
} upgrade_state_enum;
```

**为什么显式写 `= 0`**：C 语言里枚举默认从 0 开始，但显式写出来是一种「契约声明」——这个值会存进 Flash，一旦有人在中间插入新项，后面所有值都会平移，Flash 里的旧数据就会解释错。

**为什么结构体里用 `uint8_t` 而不是枚举类型**：
```c
uint8_t  active_slot;        /* 当前激活执行的分区 (boot_slot_enum) */
```
C 标准没有规定 `enum` 的底层大小（可能是 `int`，4 字节）。如果直接用枚举类型，结构体大小会随编译器变化。存进 Flash 的结构体，每个字节的位置都必须是确定的，所以用定长大小的 `uint8_t`，靠注释说明它存的是哪个枚举。

**`SLOT_APP_A = 0` 和 `SLOT_APP_B = 1` 还有一个额外好处**：可以直接当数组下标用（见 3.4 的 `slot[2]`）。

### 3.3 单槽镜像描述

`boot_param.h` L33-38：

```c
typedef struct {
    uint32_t fw_size;            /* 该槽内固件的实际字节数，0 表示该槽为空 */
    uint32_t fw_crc32;           /* 该槽内固件镜像的 CRC32 */
    uint32_t fw_version;         /* 该槽固件版本号，例如 0x010002 表示 v1.0.2 */
    uint32_t fw_flags;           /* 预留：镜像状态标志（已验证 / 待验证等） */
} boot_slot_info_t;
```

**为什么 `fw_size == 0` 表示空槽**：Flash 擦除后是全 `0xFF`，不是 0。但我们在初始化时会把整个结构体 `memset` 成 0，所以「没有写过固件」这个状态用 0 表示是明确的、不会和擦除态混淆。

**为什么 `fw_flags` 现在就留出来**：改结构体大小等于改 Flash 格式，一旦设备出厂就不能随便动。留 4 字节，将来可以放「已验证 / 待验证 / 保留给旧版本」这类标志，不用再改布局。

### 3.4 主结构体，逐字段看

`boot_param.h` L44-58：

```c
typedef struct {
    uint32_t magic;              /* 魔数: 0x544F4F42 */
    uint32_t seq_num;            /* 递增序列号，用于在日志中挑选最新一份 */

    uint8_t  active_slot;        /* 当前激活执行的分区 (boot_slot_enum) */
    uint8_t  upgrade_state;      /* 当前升级流程状态 (upgrade_state_enum) */
    uint8_t  boot_fail_count;    /* 连续启动失败计数器 */
    uint8_t  reserved1;          /* 预留字节，补齐 4 字节对齐 */

    boot_slot_info_t slot[2];    /* [0]=App-A, [1]=App-B，下标与 boot_slot_enum 一致 */

    uint32_t reserved[4];        /* 预留字段，为日后扩展保留空间 */

    uint32_t param_crc32;        /* 本结构体自身的 CRC32 (从 magic 算到 reserved 尾部) */
} boot_param_t;
```

内存布局（64 字节，无填充洞）：

| 偏移 | 字段 | 大小 | 说明 |
|---|---|---|---|
| 0 | magic | 4 | |
| 4 | seq_num | 4 | |
| 8 | active_slot | 1 | |
| 9 | upgrade_state | 1 | |
| 10 | boot_fail_count | 1 | |
| 11 | reserved1 | 1 | **手工补齐用** |
| 12 | slot[0] | 16 | App-A |
| 28 | slot[1] | 16 | App-B |
| 44 | reserved[4] | 16 | |
| 60 | param_crc32 | 4 | |

**为什么要有 `reserved1`**：三个 `uint8_t` 加起来只有 3 字节。如果不补一个字节，编译器会自动插入 1 字节填充让下一个 `uint32_t` 对齐到 4 的倍数。填充字节的内容是**不确定的**——而我们要对整个结构体算 CRC，填充字节变化会导致 CRC 变化，同一份数据算两次得到不同结果。

手工写一个 `reserved1` 并随 `memset` 一起清零，就把「不确定」变成了「确定」。

**为什么 `slot[2]` 放在 `uint8_t` 之后而不是最前面**：因为 `uint32_t` 类型的成员必须 4 字节对齐，`uint8_t` 放在前面正好凑满 4 字节（4 个）。如果反过来把 `slot[2]` 放前面，`uint32_t` 之后接 `uint8_t` 再接 `uint32_t` 会插入填充，布局就不可控了。

**为什么 `slot[2]` 而不用两个独立字段 `slot_a / slot_b`**：

```c
#define PARAM_SLOT_INFO(p, s)       ((p)->slot[(uint32_t)(s)])
```

用数组就能直接用枚举当下标访问，写成 `PARAM_SLOT_INFO(&p, SLOT_APP_B)`。如果拆成两个字段，每次访问都得写 `if (slot == A) {...} else {...}`，出错概率高得多。

### 3.5 六条布局断言，为什么 sizeof 还不够

`boot_param.h` L89-107：

```c
_Static_assert(sizeof(boot_slot_info_t) == 16U, "boot_slot_info_t layout changed");
_Static_assert(sizeof(boot_param_t) == 64U, "boot_param_t layout changed, on-flash format would break");
_Static_assert(offsetof(boot_param_t, slot) == 12U, "boot_param_t.slot must start at offset 12");
_Static_assert(offsetof(boot_param_t, param_crc32) == 60U, "boot_param_t.param_crc32 must be the last field at offset 60");
_Static_assert((sizeof(boot_param_t) % 4U) == 0U, "...");
_Static_assert(sizeof(boot_param_t) <= PARAM_SLOT_SIZE, "...");
_Static_assert((PARAM_SLOT_SIZE % 4U) == 0U, "...");
_Static_assert((PARAM_REGION_SIZE % PARAM_SLOT_SIZE) == 0U, "...");
_Static_assert((PARAM_REGION_B_ADDR + PARAM_REGION_SIZE) <= (PART_PARAM_ADDR + PART_PARAM_SIZE), "...");
```

**为什么 `sizeof` 不够**：假设有人把 `fw_size` 和 `fw_crc32` 两个字段的**位置互换**。结构体总大小还是 64 字节，`sizeof` 断言照样通过。但 Flash 里已有的数据会按旧布局解释 → 读出来的 `fw_size` 其实是 CRC，`fw_crc32` 其实是尺寸 → 镜像校验必然失败。

`offsetof` 锁的是「关键字段必须落在哪个偏移」，这种交换就会被抓住。

**这条断言实际救过一次**：最初按 56 字节写断言，编译当场失败，才发现手工累加算错了（实际 52 字节）。如果没有这条断言，会一直到上板读不到参数才发现。

**`__attribute__((packed))` 要不要加**：不需要。这个结构体所有字段都是自然对齐的，编译器不会插入额外填充（唯一的潜在填充位被 `reserved1` 手工占掉了）。加了 `packed` 反而会让字段访问变成非对齐访问，在 Cortex-M4 上性能更差，某些情况下还会出问题。

### 3.6 日志区的物理布局

`boot_param.h` L80-86：

```c
#define PARAM_REGION_SIZE           0x4000U                             /* 16 KB，单区 = 一个物理扇区 */
#define PARAM_REGION_A_ADDR         (PART_PARAM_ADDR)                   /* 0x08100000 */
#define PARAM_REGION_B_ADDR         (PART_PARAM_ADDR + PARAM_REGION_SIZE) /* 0x08104000 */

#define PARAM_SLOT_SIZE             128U
#define PARAM_SLOTS_PER_REGION      (PARAM_REGION_SIZE / PARAM_SLOT_SIZE) /* 128 */
```

物理映射：

```
Sector 12 (0x08100000, 16KB) = 日志区 A
Sector 13 (0x08104000, 16KB) = 日志区 B
Sector 14~16 (96KB)          = 未使用，留给扩展

每个区内部：
+--------+--------+--------+ ... +---------+
| 槽 0   | 槽 1   | 槽 2   |     | 槽 127  |
| 128 B  | 128 B  | 128 B  |     | 128 B   |
+--------+--------+--------+ ... +---------+
```

**为什么区大小必须是 16KB**：因为它是**一个物理扇区的大小**。擦除只能整扇区擦，区大小和扇区大小不一致就会出现「擦一半」或「擦到邻居」。第 5.5 节讲了一个真实的反例。

**为什么槽是 128 字节**：

| 槽大小 | 每区槽数 | 擦除频率 | 余量 |
|---|---|---|---|
| 512 B | 32 | 每 32 次写擦一次 | 浪费 90% |
| **128 B** | **128** | **每 128 次写擦一次** | 64 字节 |
| 64 B | 256 | 每 256 次写擦一次 | 12 字节，改结构体就爆 |

结构体 64 字节，槽 128 字节正好留一半。槽越小越省空间、擦除越少，但扩展余量不足时就得改布局——那才是真正的成本。

**为什么 `PARAM_SLOT_SIZE % 4 == 0` 也要断言**：Flash 编程是按字（4 字节）写的。槽大小不是 4 的倍数，槽起点就可能不是 4 字节对齐，写的时候会错位。

---

## 4. `boot_crc32.c` —— 为什么自己实现 CRC

### 4.1 查表怎么来的

```c
static const uint32_t g_crc32_table[256] = {
    0x00000000U, 0x77073096U, 0xEE0E612CU, 0x990951BAU,
    ...
};
```

这 256 个值不是抄来的，是用多项式 `0xEDB88320` 逐位展开生成的：

```python
for i in range(256):
    c = i
    for _ in range(8):
        c = (c >> 1) ^ 0xEDB88320 if c & 1 else c >> 1
    table.append(c)
```

**为什么必须能对上 zlib**：上位机 OTA 工具算镜像 CRC，单片机校验同一个值。两边算法不一致，校验永远不过。

**验证方法**：标准测试向量 `crc32("123456789") == 0xCBF43926`。这条写进了注释，以后任何人改实现都能立刻自测。

### 4.2 两个函数，为什么要拆开

```c
uint32_t boot_crc32_update(uint32_t crc, const uint8_t *data, uint32_t length)
{
    while (length--) {
        crc = (crc >> 8) ^ g_crc32_table[(crc ^ *data++) & 0xFFU];
    }
    return crc;
}

uint32_t boot_crc32_calc(const uint8_t *data, uint32_t length)
{
    return ~boot_crc32_update(0xFFFFFFFFU, data, length);
}
```

**为什么需要 `update` 这种「能分段喂」的形态**：

App 分区是 768KB。芯片总共 512KB SRAM（还要留给 App 的堆栈、FreeRTOS、USB 缓冲）。**物理上不可能**把整个镜像读进 RAM 再算 CRC。

只能这样：

```c
uint32_t crc = 0xFFFFFFFFU;
for (每个 1KB 块) {
    crc = boot_crc32_update(crc, chunk, 1024);
}
uint32_t final = ~crc;
```

`calc` 只是 `update` 的一次性包装，给参数校验这种小数据用，读起来更直接。

### 4.3 为什么用查表而不是位循环

| 实现 | 每字节 | 768KB 耗时（x86 实测） |
|---|---|---|
| 位循环 | 8 次迭代，约 30–40 周期 | 7.0 ms |
| 查表 | 1 次查表 + 移位异或，约 8–12 周期 | 2.0 ms |

**3.5 倍**。表占 1KB Flash，对 128KB 的 Bootloader 分区可以忽略。

参数只有 64 字节时两者都是微秒级，看不出差别；但镜像校验要跑 768KB，这时候差距就实在了。

**注意 GD32 有硬件 CRC 外设**：它的多项式是 `0x04C11DB7`、**非反射**、按 32 位字计算，结果与 zlib 完全不同。要用必须上位机也改，不能一半软一半硬。目前选软件查表，兼容性最好。

---

## 5. `boot_param.c` —— 双区追加日志的核心

这是整个系统最需要仔细读的文件。

### 5.1 CRC 覆盖范围：为什么是 `sizeof - 4`

`boot_param.c` L7：

```c
#define PARAM_CRC_SPAN  (sizeof(boot_param_t) - sizeof(uint32_t))
```

CRC 算的是「从 `magic` 开始，到 `param_crc32` 之前的全部字节」。

**为什么不能连 `param_crc32` 自己一起算**：算 CRC 的时候，CRC 字段本身还没算出来。如果把它也纳入计算范围，就变成「用结果去算结果」，永远自洽不了。

**为什么用 `sizeof - 4` 而不是 `offsetof(param_crc32)`**：两者等价，但前者少一个依赖。而且断言已经锁死了 `param_crc32` 必须是最后一个字段（L95），所以 `sizeof - 4` 就是它的偏移。

### 5.2 有效性判断：为什么先查魔数

`boot_param.c` L20-26：

```c
static bool boot_param_is_valid(const boot_param_t *param)
{
    if (param->magic != BOOT_PARAM_MAGIC) {
        return false;
    }
    return (boot_crc32_calc((const uint8_t *)param, PARAM_CRC_SPAN) == param->param_crc32);
}
```

**顺序很重要**。全新芯片或擦除后的槽是 `0xFF FF FF FF ...`，魔数是 `0xFFFFFFFF`，不等于 `0x544F4F42`，第一行就返回了。

如果反过来先算 CRC：每次扫描 256 个槽，每个都白算 60 字节 CRC。而实际上 99% 的槽都是空的。

**这是「快路径优先」的典型写法**：把最便宜、最容易否决的条件放最前面。

### 5.3 序列号比较：RFC1982 是什么

`boot_param.c` L33-36：

```c
static bool boot_param_seq_is_newer(uint32_t a, uint32_t b)
{
    return ((int32_t)(a - b) > 0);
}
```

**为什么不能直接 `a > b`**：

`seq_num` 是 32 位，一直自增会回绕：`0xFFFFFFFE → 0xFFFFFFFF → 0x00000000`。

如果用 `a > b` 比较：
- a = 0x00000001（刚回绕后的新值）
- b = 0xFFFFFFFF（回绕前的旧值）
- `a > b` 为假 → 判定 b 更新 → **选错了，读到旧数据**

`(int32_t)(a - b)` 的做法：

```
a = 1, b = 0xFFFFFFFF
a - b = 1 - 0xFFFFFFFF = 2        （无符号减法自然回绕）
(int32_t)2 > 0  → 真 → a 更新 ✓
```

差值很小（2），即使符号解释也还是正数。只有当两个序列号相差超过 `0x7FFFFFFF`（约 21 亿）时才会判错——那时早就该换芯片了。

**这就是 RFC1982 序列号算术的核心思想**：不比较绝对值，比较「距离」。TCP 的序号回绕处理用的是同一套逻辑。

### 5.4 扫描一个区：三个返回值

`boot_param.c` L39-43：

```c
typedef struct {
    int32_t  newest_slot;   /* 本区最新有效槽索引，-1 表示本区无有效数据 */
    uint32_t newest_seq;    /* 对应的 seq_num */
    int32_t  free_slot;     /* 第一个空槽索引，-1 表示本区已写满 */
} param_region_info_t;
```

**为什么需要三个字段**：读参数时只关心「最新的在哪」，写参数时只关心「空槽在哪」。一次扫描同时算出两个结果，避免读和写各扫一遍。

`boot_param.c` L61-82：

```c
static void param_region_scan(uint32_t region, param_region_info_t *info)
{
    info->newest_slot = -1;
    info->newest_seq  = 0U;
    info->free_slot   = -1;

    for (uint32_t i = 0U; i < PARAM_SLOTS_PER_REGION; i++) {
        const boot_param_t *p = param_slot_ptr(region, i);

        if (p->magic == 0xFFFFFFFFU) {
            info->free_slot = (int32_t)i;
            break;                              // ← 关键
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
```

**`break` 那一行是性能关键**：

追加式日志有一个性质——**数据一定是从槽 0 开始连续的，中间不会跳过**。所以遇到第一个空槽（`0xFF`）就说明后面全是空的，可以直接停。

如果没有这个 `break`，每次启动都要把 128 个槽全读一遍（虽然空槽在魔数检查处就返回了，但仍有 128 次内存访问）。

**注意 `magic == 0xFFFFFFFF` 而不是「整个槽全 0xFF」**：只检查第一个字就够。因为有效数据的第一个字必然是魔数 `0x544F4F42`，不可能是 `0xFFFFFFFF`。

**为什么半写坏的槽不会打断扫描**：假设写到槽 5 时掉电，槽 5 的魔数已经写进去了（不是 `0xFF`），所以不会被当成空槽；它的 CRC 校验不过，所以不会被当成有效数据。扫描继续往后，槽 6 是空的，`break`。逻辑闭环。

### 5.5 读取：两区各扫一遍，取最新

`boot_param.c` L87-118：

```c
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
        return false;
    }

    memcpy(param,
           param_slot_ptr((uint32_t)best_region, (uint32_t)info[best_region].newest_slot),
           sizeof(boot_param_t));
    return true;
}
```

**为什么读也要扫两个区**：因为不知道最新数据到底在哪个区。正常情况下在「活动区」，但掉电可能让活动区刚被擦除（切区时），这时最新数据还在另一个区。

**`best_region < 0` 的判断**：初始化为 -1，如果两个区都没有有效数据就一直是 -1，返回 false，上层会走「出厂格式化」流程。

**`memcpy` 而不是直接返回指针**：返回 Flash 里的指针，调用方可能把它当普通内存写，那就直接改 Flash 了（而且会失败）。拷一份到 RAM 是明确的所有权转移。

### 5.6 写入：三种情况，一次讲清

`boot_param.c` L135-196 是整个文件的核心。

**第一步：扫描，找出活动区**

```c
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
```

**`base_seq` 为什么要从 Flash 里读，而不是用入参的 `seq_num`**：

假设调用方手里拿的是一份过期副本（seq=50），而 Flash 里最新已经是 100。如果直接用 `50 + 1 = 51` 写入，那新数据比现有的 100 还小 → 读的时候会选中 100 → **新写的被静默忽略**。

从 Flash 读最新值再加一，物理上不可能写出比现状更小的序号。

**第二步：决定写哪里**

```c
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
```

三种情况对应三种物理动作：

| 情况 | 动作 | 耗时 | 掉电风险 |
|---|---|---|---|
| 两区皆空（首次） | 直接写 A 区槽 0 | 快 | 无（本来就没数据）|
| 活动区有空槽（99% 的情况） | 追加到下一个空槽 | 快 | 只丢这一条 |
| 活动区写满（每 128 次一次）| 擦另一区 → 写它的槽 0 | 慢（百毫秒级）| 另一区完好，可恢复 |

**`1 - active` 这个写法**：`active` 只可能是 0 或 1，`1 - active` 就是取反。比写 `(active == 0) ? 1 : 0` 简洁，也不容易写错。

**为什么擦除是安全的**：走到这个分支的前提是「活动区已满」——说明最新数据一定在活动区里。被擦的那个区必然只含更旧的数据，擦了不心疼。这是整个方案掉电安全的根基。

**第三步：写**

```c
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
```

**为什么要 `temp.magic = BOOT_PARAM_MAGIC`**：调用方可能传了一份魔数错误的参数（比如 memset 后忘了设）。这里强制纠正，保证落盘的数据一定是合法的。

**`base_seq + 1U` 为什么不处理回绕**：由 `boot_param_seq_is_newer` 的 RFC1982 逻辑处理，不需要特殊判断。这就是把复杂度收敛到一个函数的好处。

**为什么写后要回读校验**：`flash_write_data` 返回 true 只代表「编程指令执行完了」，不代表「数据真的写进去了」。Flash 编程可能因为电压不稳、位磨损等原因部分失败。回读一次 CRC 能立刻发现，而不是等到下次启动才莫名其妙读不到参数。

**代价**：多一次 60 字节的 CRC 计算，微秒级。值得。

### 5.7 交叉自检：两层真相的问题

`boot_param.c` L207-219：

```c
bool boot_param_selfcheck(void)
{
    if (flash_sector_size_get(PARAM_REGION_A_ADDR) != PARAM_REGION_SIZE) {
        return false;
    }
    if (flash_sector_size_get(PARAM_REGION_B_ADDR) != PARAM_REGION_SIZE) {
        return false;
    }

    return (PARAM_REGION_B_ADDR != PARAM_REGION_A_ADDR);
}
```

**这是在解决什么问题**：

同一个事实，被两处代码各自描述了一遍：

| 位置 | 描述 |
|---|---|
| `boot_param.h` L80 | 这个区是 16KB（宏） |
| `bsp_flash.c` L26-27 | `{0x08100000, 0x004000, ...}` 这个地址是 16KB 扇区 |

如果有人把 `PARAM_REGION_SIZE` 改成 0x8000（32KB），但 BSP 表里还是 16KB，编译器**不会报任何错**。运行时：

- 擦除只擦 16KB
- 但我们以为擦了 32KB → 后半部分旧数据还在
- 写入时从槽 128 开始写 → 落在没被擦过的区域 → 写失败或数据错乱

而且现象是「偶尔参数读不出来」，极难排查。

**为什么放在 `init` 里**：参数初始化是启动流程的第一步，配置错了就立刻停，不给后面的代码擦错扇区的机会。

**`flash_sector_size_get` 越界返回 0**，所以这个判断同时覆盖了「地址非法」的情况。

### 5.8 初始化：出厂格式化

`boot_param.c` L224-255：

```c
bool boot_param_init(boot_param_t *param)
{
    if (param == NULL) {
        return false;
    }

    if (!boot_param_selfcheck()) {
        return false;
    }

    if (boot_param_read(param)) {
        return true;
    }

    memset(param, 0, sizeof(boot_param_t));
    param->magic           = BOOT_PARAM_MAGIC;
    param->seq_num         = 0;                    /* 由 write 内部递增为 1 */
    param->active_slot     = SLOT_APP_A;
    param->upgrade_state   = UPGRADE_STATE_IDLE;
    param->boot_fail_count = 0;
    PARAM_SLOT_INFO(param, SLOT_APP_A).fw_version = 0x010000;   /* v1.0.0 */

    if (!boot_param_write(param)) {
        return false;
    }

    return boot_param_read(param);
}
```

**执行顺序不能变**：

1. `selfcheck()` — 配置错了立刻拒绝
2. `read()` — 有数据就用，**不动 Flash**
3. 都没有 → `memset` + 默认值 → `write()`
4. `read()` 回读 — 让调用方拿到真正落盘的内容

**最后那个回读为什么必须**：`write()` 内部会把 `seq_num` 从 0 改成 1。如果不回读，调用方手里的 `param` 还是 `seq_num = 0`，和 Flash 里的实际值不一致。以后用这份数据做修改再写入，就会出现「序号对不上」的隐患。

**注意 `memset` 已经把 `slot[0]` 和 `slot[1]` 全清零了**，所以只需要单独设置 A 槽的版本号。B 槽保持全 0 就是「空槽」的正确表示。

---

## 6. `boot_image.c` —— 镜像校验

### 6.1 两个小工具函数

```c
static uint32_t app_base(boot_slot_enum slot)
{
    return (slot == SLOT_APP_A) ? PART_APP_A_ADDR : PART_APP_B_ADDR;
}

static uint32_t app_size(boot_slot_enum slot)
{
    return (slot == SLOT_APP_A) ? PART_APP_A_SIZE : PART_APP_B_SIZE;
}
```

**为什么不写成两张宏**：函数有类型检查，传错参数编译器会警告。而且调试时能单步进去看。

**为什么用三元而不是数组**：`slot` 只有两个值，数组反而要额外定义一张表。

### 6.2 校验主函数

```c
boot_image_status_t boot_image_verify(const boot_param_t *param, boot_slot_enum slot)
{
    if (param == NULL) {
        return IMAGE_EMPTY;
    }
    if ((uint32_t)slot > (uint32_t)SLOT_APP_B) {
        return IMAGE_SIZE_INVALID;
    }

    uint32_t size   = PARAM_SLOT_INFO(param, slot).fw_size;
    uint32_t expect = PARAM_SLOT_INFO(param, slot).fw_crc32;

    if (size == 0U) {
        return IMAGE_EMPTY;
    }
    if (size > app_size(slot)) {
        return IMAGE_SIZE_INVALID;
    }

    uint32_t addr   = app_base(slot);
    uint32_t remain = size;
    uint32_t crc    = 0xFFFFFFFFU;

    while (remain > 0U) {
        uint32_t chunk = (remain > BOOT_IMAGE_CHUNK_SIZE) ? BOOT_IMAGE_CHUNK_SIZE : remain;
        crc = boot_crc32_update(crc, (const uint8_t *)(uintptr_t)addr, chunk);
        addr   += chunk;
        remain -= chunk;
    }

    return ((~crc) == expect) ? IMAGE_OK : IMAGE_CRC_MISMATCH;
}
```

**三道前置检查，顺序有讲究**：

1. `param == NULL` → 防御性检查
2. `slot` 越界 → 防止后面数组越界
3. `size == 0` → 空槽，**必须先于 size 上限检查**，否则 `0 > 768KB` 为假，会继续往下算 0 字节的 CRC，得到一个「合法」的空 CRC，和记录里的 0 比较——如果记录恰好也是 0（未初始化），就会误判成 OK

**`size > app_size(slot)` 这一条防的是越界读**：如果 `fw_size` 因为数据损坏变成 0xFFFFFFFF，不加这条检查就会从 App 区一直读到 Flash 末尾甚至越界，可能触发 BusFault。

**分块循环**：

```c
uint32_t chunk = (remain > BOOT_IMAGE_CHUNK_SIZE) ? BOOT_IMAGE_CHUNK_SIZE : remain;
```
最后一块可能不足 1KB，取剩余长度。`BOOT_IMAGE_CHUNK_SIZE = 1024`。

**为什么 1KB**：太小则循环开销占比高，太大则……其实无所谓，因为根本不占缓冲区——`boot_crc32_update` 是直接读 Flash 地址的，没有任何内存拷贝。1KB 只是让单次循环的粒度合理。

**直接指针读 Flash**：`(const uint8_t *)(uintptr_t)addr` 把 Flash 地址当普通指针用。Cortex-M 的 Flash 是内存映射的，可以直接按字节读，速度比调用 `HAL_FLASH_Read` 之类的函数快得多。

### 6.3 状态转文字

```c
const char *boot_image_status_str(boot_image_status_t status)
{
    switch (status) {
    case IMAGE_OK:              return "OK";
    case IMAGE_EMPTY:           return "EMPTY";
    case IMAGE_SIZE_INVALID:    return "SIZE_INVALID";
    case IMAGE_CRC_MISMATCH:    return "CRC_MISMATCH";
    default:                    return "UNKNOWN";
    }
}
```

**为什么返回 ASCII 而不是中文**：中文是 UTF-8 多字节，通过串口/USB 打印时如果上位机编码不对就是乱码。调试信息用 ASCII 最保险。

**为什么用 `switch` 而不是查表**：编译器会把 `switch` 编译成跳转表或比较链，两种都很快；而查表需要额外一张指针数组。十几个状态以下，`switch` 更直观。

---

## 7. `main.c` —— 启动流程

### 7.1 为什么参数加载排在 `systick_init()` 之前

`main.c` L83-90：

```c
int main(void)
{
    if (!boot_param_init(&g_boot_param)) {
        boot_fatal(BOOT_FATAL_PARAM_UNAVAILABLE);
    }

    systick_init();
    usb_init();
    ...
}
```

**复位后的状态**：Cortex-M 复位后中断是关闭的（`PRIMASK = 1`）。所以此刻调用 `boot_param_init()`，即使它内部要擦一个 16KB 扇区（耗时百毫秒级），也不会被任何中断打断。

**如果放在 `systick_init()` 之后**：

- SysTick 中断会周期性触发
- 中断服务程序在 Flash 里，而 Flash 正在擦除 → CPU 取指被阻塞
- 表现是「擦除时间被拉长」甚至时序异常

虽然 Cortex-M4 的设计保证不会崩，但**擦写期间有中断嵌套是排查问题的噩梦**。能在最干净的状态下做完，就别引入变量。

### 7.2 校验与分支

`main.c` L92-104：

```c
    boot_image_status_t st = boot_image_verify(&g_boot_param,
                                               (boot_slot_enum)g_boot_param.active_slot);
    g_active_image_status = st;

    if (st == IMAGE_EMPTY) {
        /* 当前槽没有固件 —— 正常的"等待升级"状态，不参与计数 */
    } else if (st != IMAGE_OK) {
        boot_handle_image_failure(st);
    }
```

**`g_active_image_status = st;` 为什么先赋值**：`boot_handle_image_failure` 内部会复位（不返回），如果只在成功路径赋值，失败时调试器里就看不到状态。先赋值保证任何路径下都能看到原因。

**`IMAGE_EMPTY` 单独处理，这是本轮最重要的修正**：

最初写的是 `if (st != IMAGE_OK) { boot_handle_image_failure(st); }`，结果：

```
出厂状态（只有 Bootloader、没有 App）
  → A 槽 fw_size = 0 → IMAGE_EMPTY
  → 被当成失败 → fail_count = 1 → 复位
  → 再来一次 → fail_count = 2 → 复位
  → 第三次 → fail_count = 3 → 查 B 槽也是空 → boot_fatal(2) → 死机
```

**USB 都起不来，串口什么都看不到**，现象是「设备上电毫无反应」。

根因是把两种完全不同的业务语义混为一谈：

| 状态 | 业务含义 | 该做什么 |
|---|---|---|
| EMPTY | 还没装固件，等待升级 | 进 USB 等上位机下发 |
| CRC_MISMATCH | 装了但坏了 | 计数、重试、回滚 |

### 7.3 失败处理：为什么用复位而不是原地重试

`main.c` L54-81：

```c
static void boot_handle_image_failure(boot_image_status_t status)
{
    g_active_image_status = status;

    if (g_boot_param.boot_fail_count < 0xFFU) {
        g_boot_param.boot_fail_count++;
    }

    if (g_boot_param.boot_fail_count >= BOOT_MAX_FAIL_COUNT) {
        boot_slot_enum alt = other_slot((boot_slot_enum)g_boot_param.active_slot);

        if (boot_image_verify(&g_boot_param, alt) == IMAGE_OK) {
            g_boot_param.active_slot     = (uint8_t)alt;
            g_boot_param.upgrade_state   = UPGRADE_STATE_ROLLBACK;
            g_boot_param.boot_fail_count = 0U;
            (void)boot_param_write(&g_boot_param);
            NVIC_SystemReset();
        }

        boot_fatal(BOOT_FATAL_NO_VALID_IMAGE);
    }

    if (!boot_param_write(&g_boot_param)) {
        boot_fatal(BOOT_FATAL_PARAM_WRITE_FAILED);
    }
    NVIC_SystemReset();
}
```

**`if (boot_fail_count < 0xFF) ++`**：饱和加法。防止计数溢出回绕到 0，那样就永远到不了阈值。

**为什么每次失败都复位，而不是在原地再试三次**：

如果写成 `for (i = 0; i < 3; i++) { if (verify() == OK) break; }`，那计数反映的是「一次启动里试了三次」，而不是「真实启动了三次」。两者在诊断上完全不同：

- 前者可能是读 Flash 的偶发错误
- 后者意味着「每次启动都失败」

复位重来的代价是时间（每次校验 768KB 约 30ms，三次不到 0.1 秒），换来的是语义正确。

**为什么回滚后要 `NVIC_SystemReset()`**：因为参数已经改成新槽了，需要让它走一遍完整的启动流程——重新加载参数、重新校验、重新跳转。原地继续执行会绕过这些步骤。

**`boot_fatal` 为什么关中断再死循环**：

```c
static void boot_fatal(uint32_t reason)
{
    g_boot_fatal_reason = reason;
    __disable_irq();
    for (;;) {
    }
}
```

关中断是为了防止中断在一个「已经判定为不可恢复」的状态下继续操作硬件（比如 USB 中断还在跑，会让调试器误以为系统还活着）。死循环是为了让调试器还能停下、看变量。

**为什么留 `g_boot_fatal_reason`**：Bootloader 没有 LED、没有屏幕。出问题时唯一的线索就是调试器里这个变量。1/2/3 分别对应三种故障，比单纯「卡住了」有用得多。

### 7.4 现在停在哪

`main.c` L106-114：

```c
    /* 走到这里表示：镜像校验通过，或者当前槽为空等待升级。
     * 第三段在这里接管：镜像有效时设置向量表偏移并跳转到 App。 */

    systick_init();
    usb_init();

    while (1) {
        usb_cdc_process();
    }
```

**当前阶段的行为**：不管校验通过还是空槽，都进 USB 循环。

这样做的目的：先在没有 App 的情况下验证前两段（参数系统 + 校验逻辑）是否正常。USB 能枚举、能回环，说明整个启动链路跑通了。

**第三段要做的事**（尚未实现）：

```c
if (st == IMAGE_OK) {
    // 1. 关中断
    // 2. 把用过的外设（USB、SysTick）恢复到复位状态
    // 3. SCB->VTOR = app_base(active_slot)
    // 4. __set_MSP(*(uint32_t*)app_base)      取 App 的栈顶
    // 5. 跳转到 *(uint32_t*)(app_base + 4)    取 Reset_Handler
}
```

**第 2 步最容易被忽略**：Bootloader 初始化了 USB，App 里再初始化一次会拿到脏状态（比如端点还配置着、中断还挂着）。跳转前必须把用过的外设彻底复位。

---

## 8. 设计是怎么一步步演变的

这一章回答「为什么现在是这个样子」。

### 8.1 参数区方案的三代

**第一代：假乒乓（原始代码）**

```c
if (!flash_erase_sector_by_addr(PART_PARAM_ADDR)) return false;
flash_write_data(PARAM_PAGE_0_ADDR, ...);
flash_write_data(PARAM_PAGE_1_ADDR, ...);
```

擦整个 128KB 扇区，然后写两页。注释写着「Ping-Pong 双页备份容灾」。

**问题**：两页在同一个扇区里，擦的时候一起死。掉电窗口约 1 秒，期间两页都是 `0xFF` → 读不到 → 回出厂默认 → `active_slot`、`upgrade_state`、`boot_fail_count` 全丢。

**为什么「双页」救不了**：Flash 的擦除单位是扇区，不是字节。参数区总大小（1KB）远小于扇区（128KB），所以「擦旧写新」必然先毁掉旧的。

> 判定规则：只要 擦除单位 > 参数区总大小，同一擦除单元内的多份副本就不可能构成容灾。

**第二代：真乒乓（两扇区交替）**

```
S12 = 副本 A    S13 = 副本 B
写入时只擦较旧的那份所在扇区，另一份全程有效
```

解决了掉电问题。但**每次写都要擦一个扇区**：

- GD32F4 扇区寿命典型 1 万次
- `boot_fail_count` 如果每次启动都落盘，一天 10 次启动 → 3 年报废

**第三代：双区追加日志（最终）**

```
Region A (16KB, 128 槽) ←→ Region B (16KB, 128 槽)
追加写不擦旧槽；写满才切区，切前擦另一区
```

| 指标 | 假乒乓 | 真乒乓 | 双区日志 |
|---|---|---|---|
| 掉电安全性 | 差（全丢） | 好 | 好 |
| 擦除频率 | 每次写 | 每次写 | 每 128 次写 |
| 实测 512 次写的擦除次数 | 512 | 512 | **4** |

### 8.2 一个被否掉的中间方案

中途考虑过：参数区 128KB 当一个区，切成 256 个 512 字节槽，写满才擦。

```
PARAM_MAX_SLOTS = 128KB / 512B = 256 个槽
擦除调用         = flash_erase_sector_by_addr(PART_PARAM_ADDR)  → 只擦 S12 的 16KB
```

**问题**：槽 32–255 的旧数据永远回收不了（它们所在的扇区从没被擦过），第 33 次写就找不到空槽，又触发一次擦除。

擦除频率从「1/256」退化成「1/32」，寿命收益缩水 8 倍。而且从代码上看完全正常，只有算过账才发现。

> 规则：日志式方案要求「逻辑区大小 == 物理擦除单元」。把区切成 N 个扇区却不擦全部，就会出现回收不了的死槽。

### 8.3 结构体为什么从 52 字节变成 64

第一版只有一组 `fw_size / fw_crc32 / fw_version`：

```
问题：这组字段描述哪个槽？
  - 描述 active_slot？切槽后 B 槽的信息就没了
  - 描述待测试槽？A 槽运行正常时又说不清

而 A/B 双槽的意义正是「两槽都能独立校验」
```

第二版改成每槽一组，用 `slot[2]` 数组 + 枚举下标。

**为什么现在改零成本**：设备还没出厂。一旦有设备在外面跑，改布局就需要写迁移逻辑（读旧格式 → 转换 → 写新格式），那才是真正的负担。

---

## 9. 工程与链接配置

### 9.1 scatter 文件

`Application/Project/MDK-ARM/FreeRTOS-temp.sct`：

```
; App-A 主运行区：0x08020000 ~ 0x080E0000 (768KB)
LR_IROM1 0x08020000 0x000C0000  {
  ER_IROM1 0x08020000 0x000C0000  {
   *.o (RESET, +First)
   ...
```

**为什么要单独指定**：Keil 工程默认用 Target 对话框里的内存布局，但这个工程用了自定义 scatter（`umfTarg=0`），所以**改 Target 对话框里的 IROM 不起作用**，必须改这个文件。

**改造前它是 `0x08010000 0x001F0000`**：App 直接链在 Bootloader 分区内部，一旦烧录就覆盖引导程序。

### 9.2 向量表偏移

`Application/Application/SRC/main.c`：

```c
    nvic_vector_table_set(NVIC_VECTTAB_FLASH, PART_APP_A_VTOR_OFFSET);
```

**为什么需要设置**：Cortex-M 复位后 `VTOR` 默认是 0，即认为向量表在 0x00000000。App 运行在 0x08020000，如果不改 VTOR，任何中断都会跳到 0x08000000 的 Bootloader 向量表 → 跑到 Bootloader 的代码里 → 崩溃。

**为什么用宏**：见 2.3 的三处地址不一致的教训。

### 9.3 Bootloader 的 IROM 限制

`Bootloader/Project/MDK-ARM/FreeRTOS-temp-BOOT.uvprojx` 里 IROM1 是 `Start 0x8000000 / Size 0x20000`（128KB）。

**为什么要限制**：如果不限（默认 2MB），Bootloader 编译不报错，但一旦固件长到超过 128KB，链接器会把它排到 App-A 的地址上 → 烧录时直接把 App 覆盖掉，而且没有任何警告。

**踩过的坑**：外部改了 uvprojx 后，Keil 还开着，一次保存就把这个限制改回 2MB 了。实测当时 Bootloader 只有 17.8KB（map 文件 `Total ROM Size = 18,252`），所以不是容量问题，是 Keil 覆盖。

> 规则：改工程文件前先让 Keil 关闭。

### 9.4 烧录擦除模式

`Options for Target → Utilities → Settings → Download Function`：

| 选项 | 后果 |
|---|---|
| Erase Full Chip | 参数区一起清空，每次下载都回出厂参数 |
| **Erase Sectors** | 只擦下载数据覆盖的扇区（Bootloader 约 19KB → 只擦 S0、S1）|
| Do not Erase | 第二次下载报错 |

**必须选 Erase Sectors**，否则测「参数持久化」「掉电恢复」时测的都不是真实现象。

**这个设置不存进工程文件**，换电脑或重装 Keil 要重设。

---

## 10. 测试是怎么写的

### 10.1 核心技巧：把 Flash 映射成普通内存

嵌入式代码难测，是因为依赖真实硬件。但参数模块是纯逻辑——只要「Flash 可读写」这个前提能满足，就能在 PC 上跑。

```c
void *m = VirtualAlloc((void *)(uintptr_t)FLASH_BASE, FLASH_SPAN,
                       MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
```

Windows 的 `VirtualAlloc` 可以在指定地址申请内存。`0x08100000` 是 135MB 处，64 位进程里通常空闲，可以直接占住。

之后 `boot_param.c` 里那些 `(const boot_param_t *)(uintptr_t)PARAM_REGION_A_ADDR` 的强转，就真的读到了我们申请的内存。

**前提**：编译的是**真实的** `boot_param.c`，不是复制一份改过的。这样才能测到实际代码。

### 10.2 模拟掉电

```c
bool flash_write_data(uint32_t addr, const uint8_t *buf, uint32_t len)
{
    if (g_power_loss) {
        memcpy(g_flash + (addr - FLASH_BASE), buf, 16U);   /* 只写 16 字节 */
        return false;
    }
    ...
}
```

`g_power_loss` 置位时，只写前 16 字节就返回失败——精确模拟「编程途中断电」这个物理场景。

这是**上板子做不到的**：真掉电是随机的，无法控制在「写了 16 字节」那一刻。仿真反而比硬件测试更可控。

### 10.3 独立实现的交叉验证

测试里的 CRC 用的是**位循环**，被测代码用的是**查表**：

```c
static uint32_t crc32_of(const uint8_t *d, uint32_t n)   /* 位循环，独立实现 */
```

两者算法相同但实现不同。如果表的 256 个值抄错了一个，两边结果就对不上，测试立刻失败。

这比「用同一个函数算两遍」有意义得多——后者永远自洽，发现不了问题。

### 10.4 43 项断言覆盖什么

| 组 | 关键验证 |
|---|---|
| S1 空片初始化 | 首次格式化，**且不触发擦除**（说明追加写生效）|
| S2 追加写 | 连写 3 次后**槽 0 的旧内容仍在**（证明真的没擦）|
| S3 写入掉电 | 半写槽损坏，read 回退到上一份 |
| S4 写满切区 | 128 次写后切区，**A 区数据完好**（证明擦的是 B 区）|
| S5 切区掉电 | 擦除成功、写入失败时，另一区仍能读出最新值 |
| S6 seq 回绕 | `0xFFFFFFFF → 0` 仍判对 |
| S7 双区全损 | read 失败，init 重建 |
| S8 扇区表自检 | 表不一致时拒绝服务 |
| S9 磨损核算 | 512 次写只擦 4 次 |
| S10 镜像校验 | 完整/篡改/空槽/超长 四类 |

### 10.5 一个差点误报的坑

某次测试代码编译失败（`PARAM_ACTIVE_INFO(&p) == &p.slot[...]` 把结构体值当指针比），但没检查退出码，直接跑了旧的 `.exe`，屏幕上全是 `ALL PASS`。

发现原因是：输出里没有新加的 `sizeof(boot_param_t) = 64` 这行。才回头查编译日志。

> 规则：跑测试前先 `rm -f` 旧产物，或者检查二进制时间戳。绿灯可能是上一次的。

---

## 11. 踩坑清单

| # | 现象 | 根因 | 解决 |
|---|---|---|---|
| 1 | 参数可能全丢 | 擦除粒度 > 参数区，双页同归于尽 | 改双区日志 |
| 2 | 扇区寿命不足 | 每次写都擦 | 追加式写入 |
| 3 | App 跑飞 | scatter / VTOR / 分区表三处地址不一致 | 统一 + 宏推算 VTOR |
| 4 | App 引不到 `boot_partition.h` | include 路径缺 `Common\Inc` | 补路径 |
| 5 | 结构体大小算错 | 手工累加（以为 56，实际 52）| `_Static_assert(sizeof == N)` 拦下 |
| 6 | 宏名撞车 | `PARAM_SLOT_SIZE` 一处指 16KB，一处指 512B | 拆成 `PARAM_REGION_SIZE` / `PARAM_SLOT_SIZE` |
| 7 | 擦除频率高 8 倍 | 逻辑区 128KB 但只擦 16KB，死槽回收不了 | 逻辑区 == 一个扇区 |
| 8 | 出厂首次上电死机 | `IMAGE_EMPTY` 被当失败，3 次后 fatal | 空槽单独处理 |
| 9 | IROM 限制消失 | Keil 保存覆盖 uvprojx | 改工程前关 Keil |
| 10 | 断言像死代码 | 只在其被 include 的翻译单元生效 | 每个工程各调一次 |
| 11 | 测试全过但没跑新代码 | 编译失败跑了旧二进制 | 删产物再跑 |

---

## 12. 速查

### 12.1 地址

| 名称 | 地址 | 大小 | 扇区 |
|---|---|---|---|
| Boot | 0x08000000 | 128 KB | S0–S4 |
| App-A | 0x08020000 | 768 KB | S5–S10 |
| User | 0x080E0000 | 128 KB | S11 |
| Param（整体）| 0x08100000 | 128 KB | S12–S16 |
| 日志区 A | 0x08100000 | 16 KB | S12 |
| 日志区 B | 0x08104000 | 16 KB | S13 |
| App-B | 0x08120000 | 768 KB | S17–S22 |
| 预留 | 0x081E0000 | 128 KB | S23 |

### 12.2 常量

| 宏 | 值 | 定义在 |
|---|---|---|
| `BOOT_PARAM_MAGIC` | 0x544F4F42 | boot_param.h:10 |
| `BOOT_MAX_FAIL_COUNT` | 3 | boot_param.h:13 |
| `PARAM_REGION_SIZE` | 0x4000 | boot_param.h:80 |
| `PARAM_SLOT_SIZE` | 128 | boot_param.h:85 |
| `PARAM_SLOTS_PER_REGION` | 128 | boot_param.h:86 |
| `BOOT_IMAGE_CHUNK_SIZE` | 1024 | boot_image.h |

### 12.3 关键函数位置

| 功能 | 文件:行 |
|---|---|
| 有效性判断 | boot_param.c:20 |
| seq 比较 | boot_param.c:33 |
| 扫区 | boot_param.c:61 |
| 读 | boot_param.c:87 |
| 写 | boot_param.c:135 |
| 自检 | boot_param.c:207 |
| 初始化 | boot_param.c:224 |
| 镜像校验 | boot_image.c boot_image_verify |
| 启动流程 | main.c:83 |
| 失败处理 | main.c:54 |

### 12.4 上板验证

| 时机 | 预期 |
|---|---|
| 首次上电 | 擦 S12 后写默认参数；`seq_num == 1`；`0x08100000` 的 magic = 0x544F4F42 |
| 第二次复位 | `seq_num` **仍是 1**（变 2 说明每次都重新格式化了）|
| 只有 Bootloader | `g_active_image_status == 1`（EMPTY），进 USB 循环 |
| USB | 设备管理器出现 CDC 串口，回环正常 |
| 反复下载 | 选 Erase Sectors 时参数 magic 仍在 |

### 12.5 调试器变量

| 变量 | 含义 |
|---|---|
| `g_boot_fatal_reason` | 0=正常；1=参数区不可用；2=两槽皆废；3=计数落盘失败 |
| `g_active_image_status` | 0=OK；1=EMPTY；2=SIZE_INVALID；3=CRC_MISMATCH |
| `g_boot_param` | 完整参数快照 |

---

## 13. 每层设计的意图（一句话版）

| 设计 | 意图 |
|---|---|
| 分区表集中在一个头文件 | 地址只有一个真相，两边都引它 |
| 静态断言 | 能在编译期抓的，绝不放到运行时 |
| 参数区放 Bank1 | 擦参数不阻塞 Bootloader 取指 |
| 双区追加日志 | 用「追加」代替「覆盖」，擦除频率降 128 倍 |
| 每槽 128 字节 | 结构体 64B，一半余量，够扩展 |
| seq + RFC1982 | 判新旧不怕回绕 |
| CRC 先查魔数 | 空槽零成本跳过 |
| 写后回读校验 | 编程失败当场发现，不留给下次启动 |
| 运行期扇区表自检 | 跨模块的两份真相必须交叉核对 |
| 四态校验结果 | 空槽是等待，损坏才是故障 |
| 复位而非原地重试 | 让计数反映真实启动次数 |
| 失败计数不清零 | 由 App 确认健康后清，否则检测不到跑飞 |
