#include "bsp_uart.h"
#include <stdio.h>

/* ======================== RX DMA 环形缓冲 ======================== */
static uint8_t s_uart0_rx_dma_buffer[UART0_RX_DMA_BUFFER_SIZE];

/* ======================== 初始化 ======================== */

void bsp_uart_init(void)
{
    dma_single_data_parameter_struct dma_init_struct = { 0 };

    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_USART0);

    /* TX: PA9 AF7 复用推挽 */
    gpio_mode_set(UART0_TX_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, UART0_TX_PIN);
    gpio_output_options_set(UART0_TX_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, UART0_TX_PIN);
    gpio_af_set(UART0_TX_PORT, GPIO_AF_7, UART0_TX_PIN);

    /* RX: PA10 AF7 复用上拉 */
    gpio_mode_set(UART0_RX_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, UART0_RX_PIN);
    gpio_af_set(UART0_RX_PORT, GPIO_AF_7, UART0_RX_PIN);

    usart_deinit(UART0_PERIPH);
    usart_baudrate_set(UART0_PERIPH, UART0_BAUDRATE);
    usart_receive_config(UART0_PERIPH, USART_RECEIVE_ENABLE);
    usart_transmit_config(UART0_PERIPH, USART_TRANSMIT_ENABLE);
    usart_enable(UART0_PERIPH);

    /* RX DMA: 循环接收 */
    rcu_periph_clock_enable(UART0_DMA_RCU);
    dma_deinit(UART0_RX_DMA, UART0_RX_DMA_CH);
    dma_init_struct.direction = DMA_PERIPH_TO_MEMORY;
    dma_init_struct.memory0_addr = (uint32_t)s_uart0_rx_dma_buffer;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init_struct.number = UART0_RX_DMA_BUFFER_SIZE;
    dma_init_struct.periph_addr = (uint32_t)(&USART_DATA(UART0_PERIPH));
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.priority = DMA_PRIORITY_MEDIUM;
    dma_init_struct.circular_mode = DMA_CIRCULAR_MODE_ENABLE;
    dma_single_data_mode_init(UART0_RX_DMA, UART0_RX_DMA_CH, &dma_init_struct);
    dma_channel_subperipheral_select(UART0_RX_DMA, UART0_RX_DMA_CH, UART0_RX_DMA_SUBPERI);
    dma_channel_enable(UART0_RX_DMA, UART0_RX_DMA_CH);
    usart_dma_receive_config(UART0_PERIPH, USART_RECEIVE_DMA_ENABLE);
}

/* ======================== 阻塞发送 ======================== */

void bsp_uart0_send_bytes(const uint8_t *buf, uint16_t len)
{
    uint16_t i;

    if ((buf == NULL) || (len == 0U)) {
        return;
    }

    for (i = 0U; i < len; i++) {
        usart_data_transmit(UART0_PERIPH, (uint8_t)buf[i]);
        while (RESET == usart_flag_get(UART0_PERIPH, USART_FLAG_TBE)) {
        }
    }
    while (RESET == usart_flag_get(UART0_PERIPH, USART_FLAG_TC)) {
    }
}

/* ======================== printf 重定向（MicroLib） ======================== */

int fputc(int ch, FILE *f)
{
    (void)f;
    usart_data_transmit(UART0_PERIPH, (uint8_t)ch);
    while (RESET == usart_flag_get(UART0_PERIPH, USART_FLAG_TBE)) {
    }
    return ch;
}
