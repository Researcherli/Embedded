#ifndef AX_USB_DRIVER_H
#define AX_USB_DRIVER_H

#include "drv_usb_core.h"

#include <stdbool.h>

#define UPDATE_TXRX_SIZE        (4096)

extern uint8_t usb_tx_buf[UPDATE_TXRX_SIZE];
extern uint32_t usb_tx_len;

void usb_init(void);
usb_core_driver *usb_cdc_acm(void);
int usb_cdc_write(uint8_t *buf, uint16_t len);
int usb_cdc_write_string(const char *s);
void usb_cdc_process(void);
uint8_t usb_is_connected(void);
void board_pack_buf_data(const char *buf, uint32_t len);
bool usb_is_configured(void);

#endif
