#include "usb_task.h"
#include "usb_driver.h"

/*!
    \brief  USB CDC 轮询任务（BOOT 原生模型）

            每 2ms 调用一次 usb_cdc_process()：
            configured 状态下收包 -> 回显（官方 cdc_acm demo 两拍模型）
*/
static void usb_task(void *pvParameters)
{
    (void)pvParameters;

    for (;;) {
        usb_cdc_process();

        vTaskDelay(pdMS_TO_TICKS(2U));
    }
}

BaseType_t usb_task_create(void)
{
    return xTaskCreate(usb_task, "usb_task", 512U, NULL,
                       (UBaseType_t)8U, NULL);
}
