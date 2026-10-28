#include "usb_driver.h"
#include "gd32f4xx.h"
#include "systick.h"

#include <string.h>

/*
 * BOOT 练习工程 - USB CDC 薄封装（裸机轮询版）
 * 收发完全走官方 V3.3.3 库的原生 API：
 *   cdc_acm_check_ready() / cdc_acm_data_receive() / cdc_acm_data_send()
 * 数据流与官方 Examples/USB/USB_Device/cdc_acm demo 的 main 循环一致。
 */

/* 官方库要求的延时接口（demo 在 gd32f4xx_hw.c 提供），适配到本工程 DWT 延时 */
void usb_udelay(const uint32_t usec)
{
    delay_us(usec);
}

void usb_mdelay(const uint32_t msec)
{
    delay_ms(msec);
}

static usb_core_driver cdc_acm_inst; /* CDC 设备核心实例（全局唯一） */

usb_core_driver *usb_cdc_acm(void)
{
    return &cdc_acm_inst;
}

/* USBFS_DM(PA11) / USBFS_DP(PA12) 复用为 USB */
static void usb_gpio_config(void)
{
    rcu_periph_clock_enable(RCU_SYSCFG);
    rcu_periph_clock_enable(RCU_GPIOA);

    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_11 | GPIO_PIN_12);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_11 | GPIO_PIN_12);
    gpio_af_set(GPIOA, GPIO_AF_10, GPIO_PIN_11 | GPIO_PIN_12);
}

/* 48MHz USB 时钟取自 PLLQ（板上系统时钟配置下的原生来源），并使能 USBFS 外设时钟 */
static void usb_rcu_config(void)
{
    rcu_pll48m_clock_config(RCU_PLL48MSRC_PLLQ);
    rcu_ck48m_clock_config(RCU_CK48MSRC_PLL48M);
    rcu_periph_clock_enable(RCU_USBFS);
}

/* PA15 控制 DP 上拉，软控制设备接入主机 */
static void usb_pull_up_enable(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);

    gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_15);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15);

    gpio_bit_set(GPIOA, GPIO_PIN_15);
}

void usb_init(void)
{
    usb_gpio_config();
    usb_rcu_config();

    usbd_init(&cdc_acm_inst, USB_CORE_ENUM_FS, &cdc_desc, &cdc_class);

    nvic_irq_enable(USBFS_IRQn, 2U, 0U);

    usb_pull_up_enable();
}

bool usb_is_configured(void)
{
    return USBD_CONFIGURED == cdc_acm_inst.dev.cur_status;
}

/*
 * 官方 cdc_acm demo 主循环的轮询版：
 *   - check_ready()==0（收到一包且发送空闲）-> 重新挂接收
 *   - 否则 -> 把收到的数据原样发回（回显）
 * 两拍完成一次回显，与 demo 的 while(1) 体逐句对应。
 */
void usb_cdc_process(void)
{
    if (!usb_is_configured()) {
        return;
    }

    if (0U == cdc_acm_check_ready(&cdc_acm_inst)) {
        cdc_acm_data_receive(&cdc_acm_inst);
    } else {
        cdc_acm_data_send(&cdc_acm_inst);
    }
}
