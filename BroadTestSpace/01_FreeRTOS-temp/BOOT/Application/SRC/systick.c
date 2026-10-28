#include "systick.h"
#include "gd32f4xx.h"

#define  DWT_CYCCNT             *(volatile unsigned int *)0xE0001004
#define  DWT_CR                 *(volatile unsigned int *)0xE0001000
#define  DEM_CR                 *(volatile unsigned int *)0xE000EDFC
#define  DEM_CR_TRCENA          (1 << 24)
#define  DWT_CR_CYCCNTENA       (1 <<  0)

volatile uint32_t SysTickMs;
uint8_t hh = 0;
uint8_t mm = 0;
uint8_t ss = 0;

static void delay_timer_init(void)
{
    DEM_CR |= (uint32_t)DEM_CR_TRCENA;
    DWT_CYCCNT = 0;
    DWT_CR |= (uint32_t)DWT_CR_CYCCNTENA;
}

void systick_init(void)
{
    /* setup systick timer for 1000Hz interrupts */
    if (SysTick_Config(SystemCoreClock / 1000U))
    {
        /* capture error */
        while (1);
    }
    /* configure the systick handler priority */
    NVIC_SetPriority(SysTick_IRQn, 0x00U);
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
    delay_timer_init();
}

void delay_us(uint32_t us)
{
    uint32_t tCnt = 0;
    uint32_t tStart = DWT_CYCCNT;
    uint32_t tDelayCnt = us * (240000000U / 1000000);

    while (tCnt < tDelayCnt)
    {
        tCnt = DWT_CYCCNT - tStart;
    }
}

void delay_ms(uint32_t ms)
{
    delay_us(1000 * ms);
}

void delay_increment(void)
{
    ++SysTickMs;
}

uint32_t TaskDelay_GetTick(void)
{
    return SysTickMs;
}
