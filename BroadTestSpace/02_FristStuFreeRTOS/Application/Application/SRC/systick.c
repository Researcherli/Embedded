#include "systick.h"
#include "gd32f4xx.h"

#define  DWT_CYCCNT             *(volatile unsigned int *)0xE0001004
#define  DWT_CR                 *(volatile unsigned int *)0xE0001000
#define  DEM_CR                 *(volatile unsigned int *)0xE000EDFC
#define  DEM_CR_TRCENA          (1 << 24)
#define  DWT_CR_CYCCNTENA       (1 <<  0)

static void delay_timer_init(void)
{
    DEM_CR |= (uint32_t)DEM_CR_TRCENA;
    DWT_CYCCNT = 0;
    DWT_CR |= (uint32_t)DWT_CR_CYCCNTENA;
}

void systick_init(void)
{
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
    if (SysTick_Config(SystemCoreClock / 1000U))
    {
        while (1);
    }
    NVIC_SetPriority(SysTick_IRQn, 0x00U);
}

void delay_us(uint32_t us)
{
    uint32_t tStart = DWT_CYCCNT;
    uint32_t ticks_per_us = SystemCoreClock / 1000000U;
    uint32_t tDelayCnt = us * ticks_per_us;

    while ((DWT_CYCCNT - tStart) < tDelayCnt);
}

void delay_ms(uint32_t ms)
{
    while (ms--)
    {
        delay_us(1000);
    }
}
