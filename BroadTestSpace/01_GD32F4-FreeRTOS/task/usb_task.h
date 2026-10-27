#ifndef USB_TASK_H
#define USB_TASK_H

#include "FreeRTOS.h"
#include "task.h"

/*!
    \brief      创建 USB CDC 任务
    \param[in]  none
    \retval     pdPASS：创建成功；pdFAIL：创建失败（堆不足）
    \note       USB 外设初始化（usb_init）在 User/usb_init.h 中声明，
                须在 vTaskStartScheduler() 之前调用
*/
BaseType_t usb_task_create(void);

#endif /* USB_TASK_H */
