#ifndef AX_USB_DRIVER_H
#define AX_USB_DRIVER_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include "FreeRTOS.h"
#include "task.h"
#include "cdc_acm_core.h"
#include "drv_usb_core.h"

#define ENABLE_USB_LOG

#define __FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : \
                      (__builtin_strrchr(__FILE__, '\\') ? __builtin_strrchr(__FILE__, '\\') + 1 : __FILE__))

#ifdef ENABLE_USB_LOG
    #define USB_LOG(format, ...) usb_printf(" [%s] : [%s] : [%d] : " format "\n", \
                                            __FILENAME__, \
                                            __FUNCTION__, \
                                            __LINE__, \
                                            ##__VA_ARGS__)
#else
    #define USB_LOG(format, ...) ((void)0)
    #warning "Note: USB_LOG is undefined/disabled! Logs will not be generated."
#endif

/* 12KB TX 环形缓冲 */
#define UPDATE_TXRX_SIZE        (12288)

/* USB IN 端点发送完成中断唤醒的目标任务句柄（usb_task 创建时注册） */
extern TaskHandle_t xSendataTaskHandle;

/* 初始化（GPIO/RCU/USBD/NVIC/上拉），须在调度器启动前调用 */
void drv_usb_init(void);
usb_core_driver *usb_cdc_acm(void);

/* 核心发送与处理 */
int usb_cdc_write(uint8_t *buf, uint16_t len);
int usb_cdc_write_string(const char *s);
void usb_cdc_process(void);
uint8_t usb_is_connected(void);
bool usb_is_configured(void);
bool usb_cdc_tx_ready(void);
uint32_t usb_cdc_tx_free_space(void);

/* 辅助：写入 TX 环形缓冲 / 格式化打印 */
void board_pack_buf_data(const char *buf, uint32_t len);
void usb_printf(const char *format, ...);

#endif
