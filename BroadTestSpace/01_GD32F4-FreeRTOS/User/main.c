#include "gd32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"

#include "usb_task.h"
#include "usb_init.h"

/*-----------------------------------------------------------*/
/*  App 外设初始化（须在调度器启动前调用）                   */
/*-----------------------------------------------------------*/
static void app_init(void)
{
    usb_init();
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
    app_init();
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
    /* Called on each RTOS tick interrupt. */
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
    /* Called when a stack overflow is detected.
       pcTaskName will be NULL if the task has no name (corrupted stack). */
    (void)xTask;
    (void)pcTaskName;
    taskDISABLE_INTERRUPTS();
    for (;;)
    {
    }
}
