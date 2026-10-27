#include "gd32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"

#include "cdc_acm_core.h" /* CDC 收发接口 */

#include "usb_task.h"
#include "usb_init.h" /* extern usb_core_driver cdc_acm（定义于 User/init/usb_init.c） */

/*-----------------------------------------------------------*/
/*  USB 任务配置                                             */
/*-----------------------------------------------------------*/
#define USB_TASK_STACK_SIZE 1024U
#define USB_TASK_PRIORITY (tskIDLE_PRIORITY + 2U)

/*!
    \brief      USB CDC 处理任务
                - 等待 PC 枚举完成后启动数据接收
                - 收到主机数据后回显
                - 周期性打印心跳
    \param[in]  param: 未使用
    \param[out] none
    \retval     none
    \note       CDC 驱动是"中断驱动 + 非阻塞"模型：
                - 发送：cdc_acm_data_send_buffer() 同步写入 TX FIFO，packet_sent 由 IN 中断置位
                - 接收：cdc_acm_data_receive() 启动一次 OUT 接收，packet_receive 由 OUT 中断置位
*/
static void usb_task(void *param)
{
    usb_cdc_handler *cdc;
    uint32_t len;
    uint32_t heartbeat = 0U;

    (void)param;

    /* 等待 PC 枚举完成（class_data 就绪后 cdc_acm_check_ready() 返回 0） */
    while (0U != cdc_acm_check_ready(&cdc_acm))
    {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    usb_printf("usb ready\r\n");

    /* 启动第一次数据接收 */
    cdc_acm_data_receive(&cdc_acm);

    for (;;)
    {
        cdc = (usb_cdc_handler *)cdc_acm.dev.class_data[CDC_COM_INTERFACE];

        /* 收到主机发来的数据：回显 */
        if ((NULL != cdc) && (1U == cdc->packet_receive))
        {
            len = cdc->receive_length;
            cdc->packet_receive = 0U;

            if (len > 0U)
            {
                /* 等待上一次发送完成后再回显 */
                while (0U != cdc_acm_check_ready(&cdc_acm))
                {
                    vTaskDelay(1);
                }
                cdc_acm_data_send_buffer(&cdc_acm, cdc->data, len);
            }

            /* 重新启动接收 */
            cdc_acm_data_receive(&cdc_acm);
        }

        /* 周期心跳：每秒打印一次（100 * 10ms） */
        if (0U == (++heartbeat % 100U))
        {
            usb_printf("tick %lu\r\n", (unsigned long)(xTaskGetTickCount() / 1000U));
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/*!
    \brief      创建 USB CDC 任务
    \param[in]  none
    \param[out] none
    \retval     pdPASS / pdFAIL
*/
BaseType_t usb_task_create(void)
{
    return xTaskCreate(usb_task, "usb", USB_TASK_STACK_SIZE, NULL, USB_TASK_PRIORITY, NULL);
}
