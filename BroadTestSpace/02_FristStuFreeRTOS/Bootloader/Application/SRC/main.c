#include "gd32f4xx.h"
#include "systick.h"
#include "boot_partition.h"
#include "boot_param.h"
#include "boot_image.h"

#include "usb_driver.h"
#include "bsp_flash.h"

STATIC_ASSERT_PARTITION();

/* 启动参数：保留为静态全局，既供镜像校验与跳转使用，
 * 也方便调试器直接查看 active_slot / upgrade_state / boot_fail_count */
static boot_param_t g_boot_param;

/* 停机原因，调试器里看这个变量即可定位故障点 */
#define BOOT_FATAL_PARAM_UNAVAILABLE    1U  /* 参数区物理损坏，或分区表与 BSP 扇区表不一致 */
#define BOOT_FATAL_NO_VALID_IMAGE       2U  /* 两个槽都没有可用镜像 */
#define BOOT_FATAL_PARAM_WRITE_FAILED   3U  /* 记录失败计数时参数写入失败 */

static volatile uint32_t g_boot_fatal_reason;
static volatile boot_image_status_t g_active_image_status;

/**
 * @brief 致命错误停机
 * @note  Bootloader 没有 LED 与串口，失败时只能停在原地，由调试器或看门狗处理
 */
static void boot_fatal(uint32_t reason)
{
    g_boot_fatal_reason = reason;
    __disable_irq();
    for (;;) {
    }
}

static boot_slot_enum other_slot(boot_slot_enum slot)
{
    return (slot == SLOT_APP_A) ? SLOT_APP_B : SLOT_APP_A;
}

/**
 * @brief 当前激活槽的镜像不可用时：累加失败计数，达阈值则回滚到另一槽
 *
 * 处理流程：
 *   1. boot_fail_count 加一；
 *   2. 未达 BOOT_MAX_FAIL_COUNT -> 落盘计数后复位重试（可能是偶发读错）；
 *   3. 达到阈值 -> 若另一槽镜像可用则切槽、清计数、落盘并复位；
 *   4. 达阈值且另一槽也不可用 -> 停机，等待调试器或 USB 救援。
 *
 * @note 这里只覆盖"镜像校验不过"这一类失败。至于"App 启动了但没在规定时间内
 *       确认健康"（软件跑飞），需要 App 侧在健康检查通过后主动清零计数，
 *       Bootloader 不能替它清零，否则死循环永远检测不出来。
 */
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
            NVIC_SystemReset();         /* 用新槽重新走一遍启动流程 */
        }

        boot_fatal(BOOT_FATAL_NO_VALID_IMAGE);
    }

    /* 未达阈值：落盘计数后复位重试 */
    if (!boot_param_write(&g_boot_param)) {
        boot_fatal(BOOT_FATAL_PARAM_WRITE_FAILED);
    }
    NVIC_SystemReset();
}

int main(void)
{
    /* 参数区加载放在最前面：此刻复位后尚未开启任何中断，扇区擦写期间不会被
     * SysTick 或 USB 中断打断，时序最简单。
     * 两个日志区都无效时，boot_param_init 内部会自动写入出厂默认值。 */
    if (!boot_param_init(&g_boot_param)) {
        boot_fatal(BOOT_FATAL_PARAM_UNAVAILABLE);
    }

    /* 校验当前激活槽的镜像 */
    boot_image_status_t st = boot_image_verify(&g_boot_param,
                                               (boot_slot_enum)g_boot_param.active_slot);
    g_active_image_status = st;

    if (st == IMAGE_EMPTY) {
        /* 当前槽没有固件（出厂状态，或刚被擦除）——这是正常的"等待升级"状态，
         * 不参与 boot_fail_count 计数，直接进入下面的 USB 等待上位机下发固件。
         * 若把它也算作失败，出厂首次上电就会连续复位三次后停机。 */
    } else if (st != IMAGE_OK) {
        /* 槽里有固件但损坏（CRC 不符 / 长度非法）：走失败计数与回滚（内部不返回） */
        boot_handle_image_failure(st);
    }

    /* 走到这里表示：镜像校验通过，或者当前槽为空等待升级。
     * 第三段在这里接管：镜像有效时设置向量表偏移并跳转到 App。 */

    systick_init();
    usb_init();

    while (1) {
        usb_cdc_process();
    }
}
