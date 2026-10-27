/*!
    \file    gd32f4xx_it.c
    \brief   interrupt service routines

    \version 2024-12-20, V3.3.1, firmware for GD32F4xx

    \note    USBFS_IRQHandler 为移植自 B0-GD32F4-usb-temp 工程的
             USB CDC 功能补充；SysTick_Handler 由 FreeRTOS 的
             FreeRTOSConfig.h 宏映射至 port.c，无需在此定义。
*/

#include "gd32f4xx_it.h"
#include "drv_usbd_int.h"

#include "usb_init.h" /* usb_core_driver cdc_acm（定义于 User/init/usb_init.c） */

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
}

void MemManage_Handler(void)
{
}

void BusFault_Handler(void)
{
}

void UsageFault_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

/*!
    \brief      USBFS 全局中断处理
    \param[in]  none
    \param[out] none
    \retval     none
    \note       SysTick_Handler 无需在此定义：FreeRTOSConfig.h 将
                xPortSysTickHandler 宏映射为 SysTick_Handler，由
                FreeRTOS/portable/ARM_CM4F/port.c 直接提供。
*/
#ifdef USE_USB_FS
void USBFS_IRQHandler(void)
{
    usbd_isr(&cdc_acm);
}
#endif /* USE_USB_FS */
