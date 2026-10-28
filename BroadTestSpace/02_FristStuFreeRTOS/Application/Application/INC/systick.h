#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>

/* 与 systick.c 实现同名：预配 1kHz SysTick + NVIC 抢占分组，
 * 调度器启动后 SysTick 由 FreeRTOS 接管，delay_* 走 DWT 不受影响 */
void systick_init(void);
void delay_us(uint32_t us);
void delay_ms(uint32_t ms);

#endif /* SYSTICK_H */
