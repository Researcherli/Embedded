#ifndef __BSP_UART_H
#define __BSP_UART_H

#include "gd32f4xx.h"

// 宏定义 UART0 参数 (PA9 TX / PA10 RX)
#define UART0_PERIPH      USART0
#define UART0_BAUDRATE    115200U
#define UART0_TX_PORT     GPIOA
#define UART0_TX_PIN      GPIO_PIN_9
#define UART0_RX_PORT     GPIOA
#define UART0_RX_PIN      GPIO_PIN_10

/* UART0 RX DMA: DMA1_CH2, SUBPERI4 */
#define UART0_DMA_RCU                 RCU_DMA1
#define UART0_RX_DMA                  DMA1
#define UART0_RX_DMA_CH               DMA_CH2
#define UART0_RX_DMA_SUBPERI          DMA_SUBPERI4
#define UART0_RX_DMA_BUFFER_SIZE      1024U

// 函数声明
void bsp_uart_init(void);
void bsp_uart0_send_bytes(const uint8_t *buf, uint16_t len);

#endif
