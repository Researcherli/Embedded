#ifndef DRV_IAP_H
#define DRV_IAP_H

#define KB                      (1024)
#define MB                      (1024 * KB)

#define TOTAL_RAM_SIZE          (512 * KB)
#define TOTAL_FLASH_SIZE        (2 * MB)

#define MCU_RAM_ADDRESS         (0x20000000)
#define MCU_FLASH_ADDRESS       (0x08000000)

#define USER_DATA_OFFSET        (128 * KB)
#define USER_DATA_ADDRESS       (MCU_FLASH_ADDRESS + TOTAL_FLASH_SIZE - USER_DATA_OFFSET)

#define APP_OFFSET              (64 * KB)
#define APP_START_ADDRESS       (MCU_FLASH_ADDRESS + APP_OFFSET)

#define APP_MAX_SIZE            (TOTAL_FLASH_SIZE - APP_OFFSET - USER_DATA_OFFSET)

#define IAP_BUF_MAX_SIZE        (2 * KB + 256)
#define APP_UPDATE_FLAG         (MCU_RAM_ADDRESS + 128 * KB)
#define STR_UPDATE_FLAG         "jcid_update_mode"

#include <stdint.h>

typedef enum {
    IAP_MODE_USB,
    IAP_MODE_WIFI
} IapMode;

typedef enum {
    IAP_IDLE,
    IAP_WAIT_BIN_INFO,
    IAP_WAIT_BIN_DATA,
    IAP_UPDATE_DONE
} IapState;

typedef enum {
    IAP_NO_ERR,
    IAP_WARN_BUF_NO_FULL,
    IAP_ERR_BIN_SIZE_OVERFLOW,
    IAP_ERR_BUF_OVERFLOW,
    IAP_ERR_BIN_INFO,
    IAP_ERR_CRC,
} IapError;

void iap_start(void);
uint8_t iap_receive_available(IapMode mode);
IapError iap_write_handle(uint8_t *buf, uint32_t len);
IapError iap_start_update(IapMode mode, uint32_t binSize);
uint8_t iap_update_percent(void);
IapMode iap_mode(void);

#endif // DRV_IAP_H
