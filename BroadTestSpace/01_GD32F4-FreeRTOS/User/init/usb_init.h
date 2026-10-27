#ifndef USB_INIT_H
#define USB_INIT_H

#include "usbd_core.h" /* usb_core_driver / usb_core_enum */

/*!
    \brief      USB CDC 设备实例
    \note       全局定义于 usb_init.c；
                gd32f4xx_it.c 与 task/usb_task.c 通过本头文件引用
*/
extern usb_core_driver cdc_acm;

/*!
    \brief      USB 外设初始化（时钟/GPIO/USB 内核/中断）
    \param[in]  none
    \param[out] none
    \retval     none
    \note       必须在 vTaskStartScheduler() 之前调用；
                优先级分组须与 usb_intr_config() 一致。
*/
void usb_init(void);

#endif /* USB_INIT_H */
