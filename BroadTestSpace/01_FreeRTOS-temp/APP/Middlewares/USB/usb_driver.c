#include "usb_driver.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"

#include "systick.h"    /* TaskDelay_GetTick */

/* ==========================================
 * 宏定义与全局变量
 * ========================================== */

#define USB_PRINTF_BUF_SIZE 256
#define MAX_PACKET          (60)

/*
 * TX 环形缓冲：写入只移动 head，读取只移动 tail，O(1) 操作，
 * 不在临界区内做 memmove。
 */
static usb_core_driver cdcAcm;
static uint8_t usbConnect;
static uint8_t usb_tx_ring[UPDATE_TXRX_SIZE];
static volatile uint32_t usb_tx_ring_head = 0U;   /* 写入位置 */
static volatile uint32_t usb_tx_ring_tail = 0U;   /* 读取位置 */
static uint8_t* usb_last_rx_buf = NULL;
static uint8_t usb_rx_snapshot[USB_CDC_DATA_PACKET_SIZE];

/* TX 丢弃计数器，方便调试 */
volatile uint32_t g_usb_tx_drop_cnt = 0U;

/* 私有函数声明 */
static uint32_t usb_tx_ring_write(const char* buf, uint32_t len);
static int cdc_acm_ft232_try_send_packet(uint8_t* data, int data_len);
static int cdc_acm_ft232_try_sending(uint8_t* data, int data_len);

/* ==========================================
 * 环形缓冲区工具
 * ========================================== */

/* 获取环形缓冲区已用字节数（可在临界区外安全读取快照） */
static inline uint32_t usb_tx_ring_pending(void)
{
    return (usb_tx_ring_head - usb_tx_ring_tail + UPDATE_TXRX_SIZE) % UPDATE_TXRX_SIZE;
}

/* 获取环形缓冲区空闲字节数（保留 1 字节区分满和空） */
static inline uint32_t usb_tx_ring_free(void)
{
    return UPDATE_TXRX_SIZE - 1U - usb_tx_ring_pending();
}

/*
 * 环形写入：满时截断并累加丢弃计数器。
 */
static uint32_t usb_tx_ring_write(const char* buf, uint32_t len)
{
    uint32_t free_space;
    uint32_t written = 0U;
    uint32_t chunk;

    if (buf == NULL || len == 0U) return 0U;

    taskENTER_CRITICAL();

    free_space = usb_tx_ring_free();
    if (len > free_space) {
        g_usb_tx_drop_cnt += (len - free_space);
        len = free_space;
    }

    if (len == 0U) {
        taskEXIT_CRITICAL();
        return 0U;
    }

    /* 写入可能需要分两段（到尾部 + 从头部） */
    chunk = UPDATE_TXRX_SIZE - usb_tx_ring_head;
    if (chunk > len) {
        chunk = len;
    }
    memcpy(&usb_tx_ring[usb_tx_ring_head], buf, chunk);
    usb_tx_ring_head = (usb_tx_ring_head + chunk) % UPDATE_TXRX_SIZE;
    written += chunk;

    if (written < len) {
        uint32_t remaining = len - written;
        memcpy(&usb_tx_ring[usb_tx_ring_head], buf + written, remaining);
        usb_tx_ring_head = (usb_tx_ring_head + remaining) % UPDATE_TXRX_SIZE;
        written += remaining;
    }

    taskEXIT_CRITICAL();
    return written;
}

/* ==========================================
 * 辅助功能函数
 * ========================================== */

void board_pack_buf_data(const char* buf, uint32_t len)
{
    (void)usb_tx_ring_write(buf, len);
}

void usb_printf(const char* format, ...)
{
    char buf[USB_PRINTF_BUF_SIZE];
    va_list args;
    int len;

    va_start(args, format);
    len = vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);

    if (len > 0) {
        board_pack_buf_data(buf, (uint32_t)len);
    }
}

/* ==========================================
 * USB 硬件配置与初始化
 * ========================================== */

static void usb_pull_up_enable(void)
{
    rcu_periph_clock_enable(RCU_GPIOC);
    gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_10);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
    gpio_bit_set(GPIOC, GPIO_PIN_10);
}

static void usb_gpio_config(void)
{
    rcu_periph_clock_enable(RCU_SYSCFG);
    rcu_periph_clock_enable(RCU_GPIOA);

    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_11 | GPIO_PIN_12);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_11 | GPIO_PIN_12);
    gpio_af_set(GPIOA, GPIO_AF_10, GPIO_PIN_11 | GPIO_PIN_12);
}

static void usb_rcu_config(void)
{
    rcu_pll48m_clock_config(RCU_PLL48MSRC_PLLQ);
    rcu_ck48m_clock_config(RCU_CK48MSRC_PLL48M);
    rcu_periph_clock_enable(RCU_USBFS);
}

void drv_usb_init(void)
{
    usb_gpio_config();
    usb_rcu_config();
    usbd_init(usb_cdc_acm(), USB_CORE_ENUM_FS, &cdc_desc, &cdc_class);
    nvic_irq_enable(USBFS_IRQn, 6, 0);
    usb_pull_up_enable();
}

usb_core_driver* usb_cdc_acm(void)
{
    return &cdcAcm;
}

uint8_t usb_is_connected(void)
{
    return usbConnect;
}

bool usb_is_configured(void)
{
    return USBD_CONFIGURED == usb_cdc_acm()->dev.cur_status;
}

bool usb_cdc_tx_ready(void)
{
    return usb_is_configured() && cdc_acm_ft232_check_ready(usb_cdc_acm());
}

uint32_t usb_cdc_tx_free_space(void)
{
    uint32_t free_space;
    taskENTER_CRITICAL();
    free_space = usb_tx_ring_free();
    taskEXIT_CRITICAL();
    return free_space;
}

/* ==========================================
 * 核心发送逻辑
 * ========================================== */

/*
 * 发送前等 ready（带 250 tick 超时），超时返回 -1。
 * 等待期间让出 CPU，避免空转。
 */
static int cdc_acm_ft232_send_packet(uint8_t* data, int data_len)
{
    uint32_t timeout = 0;

    if ((data_len > MAX_PACKET) || (data_len <= 0)) {
        return -1;
    }

    timeout = TaskDelay_GetTick();
    while (!cdc_acm_ft232_check_ready(usb_cdc_acm())) {
        if (USBD_CONFIGURED != usb_cdc_acm()->dev.cur_status) {
            return -1;
        }
        if ((TaskDelay_GetTick() - timeout) > 250) {
            return -1;
        }
        if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
            vTaskDelay(1);
        }
    }

    return cdc_acm_ft232_send(usb_cdc_acm(), data, data_len);
}

static int cdc_acm_ft232_try_send_packet(uint8_t* data, int data_len)
{
    if ((data_len > MAX_PACKET) || (data_len <= 0)) {
        return -1;
    }

    if (!usb_cdc_tx_ready()) {
        return 0;
    }

    return cdc_acm_ft232_send(usb_cdc_acm(), data, data_len);
}

static int cdc_acm_ft232_try_sending(uint8_t* data, int data_len)
{
    int packet_len;

    if (data_len <= 0) {
        return -1;
    }

    packet_len = USB_MIN(data_len, MAX_PACKET);
    return cdc_acm_ft232_try_send_packet(data, packet_len);
}

/*
 * 从 ring 的 tail 位置取出数据，循环发送直到 USB 不 ready 或缓冲清空。
 * 每轮最多发 MAX_FLUSH_PACKETS 包，防止异常时阻塞过久。
 * 快速路径：硬件就绪 → 非阻塞发送 (0 延迟)。
 * 慢速路径：硬件忙 (packet_sent==0) → 阻塞等待 IN 端点完成中断的
 *           任务通知唤醒，而非忙等轮询。带 2ms 超时兜底，
 *           防止中断异常时永久阻塞。
 */
#define MAX_FLUSH_PACKETS  64
#define TX_READY_WAIT_MS   2U      /* 阻塞等待 packet_sent 的超时 */

static void board_write_to_usb(void)
{
    int loop_count;

    for (loop_count = 0; loop_count < MAX_FLUSH_PACKETS; loop_count++) {
        uint32_t pending;
        uint32_t chunk;
        uint32_t current_tail;
        int      writes;

        taskENTER_CRITICAL();
        pending = usb_tx_ring_pending();
        current_tail = usb_tx_ring_tail;
        taskEXIT_CRITICAL();

        if (pending == 0U) {
            return;
        }

        chunk = UPDATE_TXRX_SIZE - current_tail;
        if (chunk > pending) {
            chunk = pending;
        }

        /* 快速路径：硬件就绪则直接非阻塞发送 */
        writes = cdc_acm_ft232_try_sending(&usb_tx_ring[current_tail], (int)chunk);

        if (writes == 0 && usb_is_configured()) {
            /* 慢速路径：硬件忙（上一包在 USB 总线上传输中）。
             * 清掉可能已积累的 stale 通知，再阻塞等 IN 完成中断唤醒。
             * pdTRUE = 取出后清零，作为二值唤醒使用。
             *
             * 注意：清零与下面 check_ready() 之间若发生 IN 完成中断，
             * 本次通知会被丢弃。但随后 check_ready() 会读到 packet_sent==1
             * 并直接发送，不会导致数据丢失；若仍忙，2ms 超时兜底会再次等待。 */
            (void)ulTaskNotifyTake(pdTRUE, 0U);
            if (!usb_is_configured()) {
                return;  /* USB 断开，立即退出 */
            }
            if (cdc_acm_ft232_check_ready(usb_cdc_acm())) {
                writes = cdc_acm_ft232_send(usb_cdc_acm(),
                    &usb_tx_ring[current_tail],
                    USB_MIN((int)chunk, MAX_PACKET));
            } else {
                /* 仍忙 → 阻塞等通知，最多 TX_READY_WAIT_MS（兜底防中断丢失） */
                (void)ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(TX_READY_WAIT_MS));
                if (!usb_is_configured()) {
                    return;
                }
                if (cdc_acm_ft232_check_ready(usb_cdc_acm())) {
                    writes = cdc_acm_ft232_send(usb_cdc_acm(),
                        &usb_tx_ring[current_tail],
                        USB_MIN((int)chunk, MAX_PACKET));
                }
            }
        }

        if (writes <= 0) {
            return;
        }

        taskENTER_CRITICAL();
        usb_tx_ring_tail = (usb_tx_ring_tail + (uint32_t)writes) % UPDATE_TXRX_SIZE;
        taskEXIT_CRITICAL();
    }
}

/* 对外写入接口（写入 TX 环形缓冲，由 usb_cdc_process 刷出） */
int usb_cdc_write(uint8_t* buf, uint16_t len)
{
    return (int)usb_tx_ring_write((char*)buf, len);
}

int usb_cdc_write_string(const char* s)
{
    return usb_cdc_write((uint8_t*)s, (uint16_t)strlen(s));
}

/* ==========================================
 * 接收处理逻辑
 * ========================================== */

static uint16_t usb_cdc_read(void)
{
    usb_cdc_handler* cdc = (usb_cdc_handler*)usb_cdc_acm()->dev.class_data[CDC_COM_INTERFACE];
    uint16_t len;

    if (cdc == NULL) {
        return 0U;
    }

    if (1 == cdc->packet_receive) {
        cdc->packet_receive = 0;
        len = cdc->receive_length;
        if (len > USB_CDC_DATA_PACKET_SIZE) {
            len = USB_CDC_DATA_PACKET_SIZE;
        }
        if (len > 0U) {
            memcpy(usb_rx_snapshot, (uint8_t*)cdc->data_rcv, len);
            usb_last_rx_buf = usb_rx_snapshot;
        }
        else {
            usb_last_rx_buf = NULL;
        }
        usbd_ep_recev(usb_cdc_acm(), CDC_DATA_OUT_EP, (uint8_t*)cdc->data_rcv, USB_CDC_DATA_PACKET_SIZE);
        return len;
    }

    return 0;
}

/* ==========================================
 * 主任务处理
 * ========================================== */

void usb_cdc_process(void)
{
    uint16_t len = usb_cdc_read();

    /* 宿主机下发的数据在 usb_rx_snapshot 中（长度 len）。
       temp 骨架无业务，收到即丢弃；业务接收处理在此处挂接。 */
    (void)len;

    board_write_to_usb();

    /* ========== 连接状态监测 ==========
     * "已连接" = USB 物理已配置 (Configured) 且 DTR 已置位。
     * DTR 由宿主机关闭 COM 口时清零 (SET_CONTROL_LINE_STATE wValue bit 0)。
     */
    {
        usb_cdc_handler* cdc = (usb_cdc_handler*)usb_cdc_acm()->dev.class_data[CDC_COM_INTERFACE];
        bool dtr_asserted = (cdc != NULL) && (cdc->dtr_enable != 0U);
        bool usb_phys_ok  = usb_is_configured();
        bool now_connected = usb_phys_ok && dtr_asserted;

        if (now_connected) {
            /* 0→1 转换：断开→重连，清理残留数据 */
            if (!usbConnect) {
                usbConnect = 1U;
                taskENTER_CRITICAL();
                usb_tx_ring_head = 0U;
                usb_tx_ring_tail = 0U;
                taskEXIT_CRITICAL();
                g_usb_tx_drop_cnt = 0U;
            }
        } else {
            /* 1→0 转换：已连接→断开，清理缓冲 */
            if (usbConnect != 0U) {
                usbConnect = 0U;
                taskENTER_CRITICAL();
                usb_tx_ring_head = 0U;
                usb_tx_ring_tail = 0U;
                taskEXIT_CRITICAL();
                g_usb_tx_drop_cnt = 0U;
            }
        }
    }

    /* 心跳：20ms 一次 keep-alive */
    {
        static uint32_t heartbeat_tick = 0UL;
        uint32_t now_tick = TaskDelay_GetTick();
        if (usb_cdc_tx_ready() && ((now_tick - heartbeat_tick) > 20U)) {
            cdc_acm_ft232_heartbeat(usb_cdc_acm());
            heartbeat_tick = now_tick;
        }
    }
}
