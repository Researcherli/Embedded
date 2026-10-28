#ifndef USB_TASK_H
#define USB_TASK_H

#include "FreeRTOS.h"
#include "task.h"

/*!
    \brief      创建 USB CDC 处理任务
    \param[in]  none
    \retval     pdPASS：创建成功；pdFAIL：创建失败（堆不足）
    \note       任务句柄写入 xSendataTaskHandle（usb_driver.h 声明），
                USB IN 端点发送完成中断通过它唤醒本任务；
                USB 外设初始化（drv_usb_init）须在 vTaskStartScheduler() 前调用
*/
BaseType_t usb_task_create(void);

#endif /* USB_TASK_H */
