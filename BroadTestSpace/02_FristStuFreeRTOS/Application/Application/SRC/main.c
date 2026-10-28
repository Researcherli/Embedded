#include "gd32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"

#include "boot_partition.h"
#include "systick.h"
#include "bsp_uart.h"
#include "usb_driver.h"
#include "usb_task.h"

/*-----------------------------------------------------------*/
/*  App 外设初始化（须在调度器启动前调用）                    */
/*-----------------------------------------------------------*/
static void app_init(void)
{
    /* 向量表偏移必须与链接地址保持一致，这里直接由分区表推算，
     * 避免以后改分区表时漏改这个硬编码常量。
     * 当前 App 固定链接并运行在 App-A；若将来改为"直接在 App-B 原地
     * 运行"的方案，需要按 param.active_slot 动态选择偏移。 */
    nvic_vector_table_set(NVIC_VECTTAB_FLASH, PART_APP_A_VTOR_OFFSET);

    systick_init();
    bsp_uart_init();
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
