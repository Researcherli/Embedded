#ifndef USB_DRIVER_H
#define USB_DRIVER_H

#include "drv_usb_core.h"
#include "cdc_acm_core.h"

#include <stdbool.h>

/*
 * BOOT 练习工程 - USB CDC 薄封装
 * USB 库已整体移植为 GD32 官方 V3.3.3 原版（Middlewares/USB/GD32F4xx_usb_library，
 * 目录结构与官方固件库一致），本层只做三件事：
 *   1. usb_init()          硬件引脚/时钟/协议栈初始化
 *   2. usb_cdc_process()   主循环轮询，按官方 cdc_acm demo 模型收发回显
 *   3. usb_cdc_acm()       设备实例访问器（gd32f4xx_it.c 的 usbd_isr 使用）
 */

/* 获取 CDC 设备核心实例（全局唯一） */
usb_core_driver *usb_cdc_acm(void);

/* GPIO / RCU / usbd_init / NVIC / DP 上拉，一次到位 */
void usb_init(void);

/* 主循环轮询：configured 状态下收包 -> 回显（官方 demo 两拍模型） */
void usb_cdc_process(void);

/* 主机是否已完成配置（SET_CONFIGURATION 后为 true） */
bool usb_is_configured(void);

#endif /* USB_DRIVER_H */
