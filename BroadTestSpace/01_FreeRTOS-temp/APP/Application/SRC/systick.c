#include "systick.h"
#include "gd32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"

#define  DWT_CYCCNT             *(volatile unsigned int *)0xE0001004
#define  DWT_CR                 *(volatile unsigned int *)0xE0001000
#define  DEM_CR                 *(volatile unsigned int *)0xE000EDFC
#define  DEM_CR_TRCENA          (1 << 24)
#define  DWT_CR_CYCCNTENA       (1 <<  0)

volatile uint32_t SysTickMs;

static void delay_timer_init(void)
{
    DEM_CR |= (uint32_t)DEM_CR_TRCENA;
    DWT_CYCCNT = 0;
    DWT_CR |= (uint32_t)DWT_CR_CYCCNTENA;
}

void SysTick_Init(void)
{
    if (SysTick_Config(SystemCoreClock / 1000U)) {
        /* capture error */
        while (1);
    }
    NVIC_SetPriority(SysTick_IRQn, 0x0FU);
    delay_timer_init();
}

void delay_us(uint32_t us)
{
    uint32_t tCnt = 0;
    uint32_t tStart = DWT_CYCCNT;
    uint32_t tDelayCnt = us * (SystemCoreClock / 1000000U);

    while (tCnt < tDelayCnt) {
        tCnt = DWT_CYCCNT - tStart;
    }
}

void delay_ms(uint32_t ms)
{
    if (ms == 0) {
        return;
    }
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        vTaskDelay(pdMS_TO_TICKS(ms));
    }
    else {
        for (uint32_t i = 0; i < ms; i++) {
            delay_us(1000);
        }
    }
}

/* 由 vApplicationTickHook 每 1ms 调用一次，供延时/超时基准使用 */
void delay_increment(void)
{
    ++SysTickMs;
}

uint32_t TaskDelay_GetTick(void)
{
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        return xTaskGetTickCount();
    }
    return 0;
}
