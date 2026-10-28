#include "gd32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"

#include "systick.h"
#include "bsp_uart.h"
#include "usb_driver.h"
#include "usb_task.h"

/*-----------------------------------------------------------*/
/* USB IN 端点发送完成中断唤醒的目标任务句柄。
   cdc_acm_core.c 在中断中通过 xTaskNotifyFromISR 唤醒它，
   由 usb_task_create() 注册。                                */
/*-----------------------------------------------------------*/
TaskHandle_t xSendataTaskHandle = NULL;

/*-----------------------------------------------------------*/
/*  App 外设初始化（须在调度器启动前调用）                    */
/*-----------------------------------------------------------*/
static void app_init(void)
{
    /* 板载 bootloader 占用前 64KB Flash，APP 从 0x08010000 起 */
    nvic_vector_table_set(NVIC_VECTTAB_FLASH, 0x10000);

    SysTick_Init();
    bsp_uart_init();
    drv_usb_init();
}

/*-----------------------------------------------------------*/
/*  创建 RTOS 任务                                            */
/*-----------------------------------------------------------*/
static void app_task_create(void)
{
    usb_task_create();
}

/*-----------------------------------------------------------*/
/*  Main                                                     */
/*-----------------------------------------------------------*/
int main(void)
{
    __disable_irq();
    app_init();
    __enable_irq();

    app_task_create();

    vTaskStartScheduler();

    for (;;)
    {
    }
}

/*-----------------------------------------------------------*/
/* FreeRTOS Hook Functions                                    */
/*-----------------------------------------------------------*/

void vApplicationTickHook(void)
{
    /* DWT 微秒延时计数依赖 1ms 节拍累计 */
    delay_increment();
}

void vApplicationMallocFailedHook(void)
{
    /* Called when pvPortMalloc() returns NULL. */
    taskDISABLE_INTERRUPTS();
    for (;;)
    {
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask,
                                   char *pcTaskName)
{
    /* Called on stack overflow. pcTaskName is NULL on corrupted stack. */
    (void)xTask;
    (void)pcTaskName;
    taskDISABLE_INTERRUPTS();
    for (;;)
    {
    }
}
