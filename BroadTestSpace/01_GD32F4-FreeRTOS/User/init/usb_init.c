#include "gd32f4xx.h"

#include "drv_usb_hw.h"   /* usb_rcu_config / usb_gpio_config / usb_intr_config */
#include "usbd_core.h"    /* usbd_init */
#include "cdc_acm_core.h" /* cdc_desc / cdc_class */

#include "usb_init.h"

/*-----------------------------------------------------------*/
/*  USB CDC 设备实例                                         */
/*  （gd32f4xx_it.c 与 task/usb_task.c 通过 extern 引用）     */
/*-----------------------------------------------------------*/
usb_core_driver cdc_acm;

/*!
    \brief      USB 外设初始化
    \param[in]  none
    \param[out] none
    \retval     none
    \note       在 vTaskStartScheduler() 之前调用；
                优先级分组须与 usb_intr_config() 一致。
*/
void usb_init(void)
{
    nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);

    usb_rcu_config();
    usb_gpio_config();
    usbd_init(&cdc_acm, USB_CORE_ENUM_FS, &cdc_desc, &cdc_class);
    usb_intr_config();
}
