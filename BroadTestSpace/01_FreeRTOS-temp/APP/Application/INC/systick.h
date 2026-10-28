#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>

extern volatile uint32_t SysTickMs;

void SysTick_Init(void);
void delay_us(uint32_t us);
void delay_ms(uint32_t ms);
void delay_increment(void);
uint32_t TaskDelay_GetTick(void);

#endif /* SYSTICK_H */
