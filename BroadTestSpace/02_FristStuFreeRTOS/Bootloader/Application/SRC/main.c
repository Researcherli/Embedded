#include "gd32f4xx.h"
#include "systick.h"
#include "boot_partition.h"
#include "boot_param.h"

#include "usb_driver.h"
#include "bsp_flash.h"

STATIC_ASSERT_PARTITION();

/* 启动参数：保留为静态全局，既供后续镜像校验与跳转使用，
 * 也方便调试器直接查看当前 active_slot / upgrade_state / boot_fail_count */
static boot_param_t g_boot_param;

/* 停机原因，调试器里看这个变量即可定位故障点 */
#define BOOT_FATAL_PARAM_UNAVAILABLE    1U  /* 参数区物理损坏，或分区表与 BSP 扇区表不一致 */

static volatile uint32_t g_boot_fatal_reason;

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

int main(void)
{
    /* 参数区加载放在最前面：此刻复位后尚未开启任何中断，扇区擦写期间不会被
     * SysTick 或 USB 中断打断，时序最简单。
     * 两个槽都无效时，boot_param_init 内部会自动写入出厂默认值。 */
    if (!boot_param_init(&g_boot_param)) {
        boot_fatal(BOOT_FATAL_PARAM_UNAVAILABLE);
    }

    systick_init();
    usb_init();

    while (1) {
        usb_cdc_process();
    }
}
