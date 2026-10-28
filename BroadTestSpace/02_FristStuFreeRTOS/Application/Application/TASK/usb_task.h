#ifndef USB_TASK_H
#define USB_TASK_H

#include "FreeRTOS.h"
#include "task.h"

/*!
    \brief      创建 USB CDC 轮询任务
    \param[in]  none
    \retval     pdPASS：创建成功；pdFAIL：创建失败（堆不足）
    \note       任务内每 2ms 调用一次 usb_cdc_process()（BOOT 原生轮询模型）；
                USB 外设初始化（usb_init）须在 vTaskStartScheduler() 前调用
*/
BaseType_t usb_task_create(void);

#endif /* USB_TASK_H */
