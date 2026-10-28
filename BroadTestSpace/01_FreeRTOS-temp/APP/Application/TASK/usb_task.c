#include "usb_task.h"
#include "usb_driver.h"
#include "bsp_uart.h"
#include <stdio.h>

/*!
    \brief  USB CDC 任务主循环

            - 每 2ms 调用一次 usb_cdc_process()：
              接收读取 → TX 环形缓冲刷写 → DTR 连接监测 → keep-alive
            - 每 2s 双通道输出一次心跳：USB CDC（usb_printf）+ 调试串口（printf）
*/
static void usb_task(void *pvParameters)
{
    (void)pvParameters;

    TickType_t last_beat = xTaskGetTickCount();

    for (;;) {
        usb_cdc_process();

        TickType_t now = xTaskGetTickCount();
        if ((now - last_beat) >= pdMS_TO_TICKS(2000U)) {
            last_beat = now;
            printf("FreeRTOS-temp alive, tick=%lu\r\n", (unsigned long)now);
            usb_printf("FreeRTOS-temp alive, tick=%lu\r\n", (unsigned long)now);
        }

        vTaskDelay(pdMS_TO_TICKS(2U));
    }
}

BaseType_t usb_task_create(void)
{
    /* 句柄写入 xSendataTaskHandle：USB IN 完成中断经它 xTaskNotifyFromISR 唤醒 */
    return xTaskCreate(usb_task, "usb_task", 512U, NULL,
                       (UBaseType_t)8U, &xSendataTaskHandle);
}
