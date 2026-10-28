#include "app_iap.h"
#include "gd32f4xx.h"
#include <string.h>
#include "usb_driver.h"
#include "valdefine.h"
#include "device.h"
#include "utility.h"
#include "bsp_flash.h"
#include "systick.h"
#include "addr.h"

#define BIN_INFO_SIZE_OFFSET    (32 * 3)
#define BIN_INFO_CRC_OFFSET     (BIN_INFO_SIZE_OFFSET + 4)

typedef struct {
    uint8_t logo[32];
    uint8_t type[32];
    uint8_t vers[32];
    uint32_t size;
    uint32_t crc;
} BinInfo;

typedef struct {
    IapState state;
    IapMode mode;
    uint32_t flashAddr;
    uint32_t eraseAddr;
    uint32_t bufIdx;
    uint8_t buf[IAP_BUF_MAX_SIZE];
    BinInfo bin;
} Iap;

static Iap iap;

/**
 * 跳转到APP
 * @return
 */
typedef void (*app_func)(void);
static app_func application;
static uint32_t app_address;
static void iap_jump_to_app(void)
{
    if (((*(__IO uint32_t*)APP_START_ADDRESS) & 0x2FF00000U) == 0x20000000U) {
        app_address = *(__IO uint32_t*)(APP_START_ADDRESS + 4U);
        application = (app_func) app_address;

        /* initialize user application's stack pointer */
        __set_MSP(*(__IO uint32_t*)APP_START_ADDRESS);

        /* jump to user application */
        application();
    }
}

/**
 * 校验APP固件信息
 * @return
 */
static uint8_t iap_check_firmware(void)
{
    static uint8_t *buf = P8_ADDR(BIN_INFO_ADDRESS);
    static BinInfo *pBinInfo = (BinInfo *)BIN_INFO_ADDRESS;

    if (0 != memcmp(pBinInfo->logo, JCID_DEVICE_LOGO, strlen(JCID_DEVICE_LOGO)))
    {
        return 0;
    }

    if (0 != memcmp(pBinInfo->type, JCID_DEVICE_TYPE, strlen(JCID_DEVICE_TYPE)))
    {
        return 0;
    }

    uint32_t binSize = BYTE_1ST_DATA(buf[BIN_INFO_SIZE_OFFSET]) + BYTE_2ND_DATA(buf[BIN_INFO_SIZE_OFFSET + 1]) + BYTE_3RD_DATA(buf[BIN_INFO_SIZE_OFFSET + 2]) + BYTE_4TH_DATA(buf[BIN_INFO_SIZE_OFFSET + 3]);
    if (binSize > APP_MAX_SIZE)
    {
        return 0;
    }

    uint16_t crc = BYTE_3RD_DATA(buf[BIN_INFO_CRC_OFFSET + 0]) + BYTE_4TH_DATA(buf[BIN_INFO_CRC_OFFSET + 1]);
    if (crc != crc16(P8_ADDR(APP_START_ADDRESS), binSize))
    {
        return 0;
    }

    return 1;
}

/**
 * 是否满足升级条件 1.有升级标志 2.校验当前flash内的APP固件
 * @return
 */
static uint8_t iap_need_update(void)
{
    uint8_t *flag = (uint8_t *)(APP_UPDATE_FLAG);
    if (0 == memcmp(flag, STR_UPDATE_FLAG, strlen(STR_UPDATE_FLAG)))
        return 1;
//    if (0 == iap_check_firmware())
//        return 1;
    return 0;
}

static IapError iap_error(IapError error)
{
    memset(&iap, 0, sizeof(Iap));
    return error;
}

/**
 * 返回已经写入flash的大小
 * @return
 */
static uint32_t iap_write_flash_size(void)
{
    return (iap.flashAddr - APP_START_ADDRESS);
}

static uint32_t iap_prepare_flash_area(uint32_t startAddr, uint32_t binSize)
{
    uint32_t eraseAddr = startAddr;
    uint32_t endAddr = startAddr + binSize;

    while (eraseAddr < endAddr)
    {
        uint32_t sectorSize = flash_sector_size_get(eraseAddr);
        flash_erase_sector(eraseAddr);
        eraseAddr += sectorSize;
    }

    return eraseAddr;
}

static IapError iap_write_bin(uint32_t bufOffset)
{
    uint32_t len = iap.bufIdx - bufOffset;
    uint8_t remain = len & 0x03; /* 长度对4取余 */
    uint32_t writes = len - remain; /* 实际写入长度要4字节对齐 */
    uint32_t flashEndAddr = iap.flashAddr + writes; /* 实际写入的末尾地址 */
    if (flashEndAddr >= iap.eraseAddr)
    {
        flash_erase_sector(flashEndAddr);
        iap.eraseAddr += flash_sector_size_get(flashEndAddr);
    }
    flash_write_data(iap.flashAddr, &iap.buf[0 + bufOffset], writes);
    iap.flashAddr += writes;

    iap.bufIdx = remain;
    memcpy(&iap.buf[0], &iap.buf[0 + bufOffset + writes], remain);

    return IAP_NO_ERR;
}

static IapError iap_write_bin_info(void)
{
    iap_write_bin(0);

    uint16_t crc = crc16((uint8_t *)APP_START_ADDRESS, iap.bin.size);
    if (crc != iap.bin.crc)
        return iap_error(IAP_ERR_CRC);

    uint8_t tmpBin[BIN_INFO_SIZE];
    memset(&tmpBin[0], 0xFF, BIN_INFO_SIZE);
    memcpy(&tmpBin[0], &iap.bin, BIN_INFO_SIZE_OFFSET);

    tmpBin[BIN_INFO_SIZE_OFFSET + 0] = iap.bin.size >> 24;
    tmpBin[BIN_INFO_SIZE_OFFSET + 1] = iap.bin.size >> 16;
    tmpBin[BIN_INFO_SIZE_OFFSET + 2] = iap.bin.size >> 8;
    tmpBin[BIN_INFO_SIZE_OFFSET + 3] = iap.bin.size >> 0;

    tmpBin[BIN_INFO_CRC_OFFSET + 0] = iap.bin.crc >> 8;
    tmpBin[BIN_INFO_CRC_OFFSET + 1] = iap.bin.crc >> 0;

    flash_erase_sector(BIN_INFO_ADDRESS);
    flash_write_data(BIN_INFO_ADDRESS, tmpBin, BIN_INFO_SIZE);

    iap.state = IAP_UPDATE_DONE;
    return IAP_NO_ERR;
}

/**
 * 校验buf接收到的bin info
 * @return
 */
static IapError iap_check_bin_info(void)
{
    uint32_t idx = 0;

    memcpy(&iap.bin, &iap.buf[idx], BIN_INFO_SIZE_OFFSET);

    if (0 != memcmp(&iap.bin.logo, JCID_DEVICE_LOGO, strlen(JCID_DEVICE_LOGO))
        || 0 != memcmp(&iap.bin.type, JCID_DEVICE_TYPE, strlen(JCID_DEVICE_TYPE)))
    {
        return iap_error(IAP_ERR_BIN_INFO);
    }
    idx += BIN_INFO_SIZE_OFFSET;

    iap.bin.size = (iap.buf[idx] << 24U) + (iap.buf[idx + 1] << 16U) + (iap.buf[idx + 2] << 8U) + iap.buf[idx + 3];
    if (iap.bin.size > APP_MAX_SIZE)
        return iap_error(IAP_ERR_BIN_SIZE_OVERFLOW);
    idx += 4;

    iap.bin.crc = (iap.buf[idx] << 8U) + iap.buf[idx + 1];

    iap.state = IAP_WAIT_BIN_DATA;
    return iap_write_bin(BIN_INFO_SIZE);
}

void iap_start(void)
{
    if (1 == iap_need_update()) {
        usb_init();

        while (iap.state != IAP_UPDATE_DONE)
        {
            usb_cdc_process();
        }

        uint8_t *flag = (uint8_t *)(APP_UPDATE_FLAG);
        memset(flag, 0, strlen(STR_UPDATE_FLAG));
		
        NVIC_SystemReset();
    }

    iap_jump_to_app();
}

/**
 * 接收数据是否有效
 * @return
 */
uint8_t iap_receive_available(IapMode mode)
{
    return (iap.mode == mode) && (iap.state > IAP_IDLE) && (iap.state < IAP_UPDATE_DONE);
}

/**
 * 接收到app固件数据后写入
 * @param buf
 * @param len
 * @return
 */
IapError iap_write_handle(uint8_t *buf, uint32_t len)
{
    if ((iap.bufIdx + len) > IAP_BUF_MAX_SIZE)
        return iap_error(IAP_ERR_BUF_OVERFLOW);

    memcpy(&(iap.buf[iap.bufIdx]), buf, len);
    iap.bufIdx += len;

    if (iap.state == IAP_WAIT_BIN_DATA)
    {
        if ((iap_write_flash_size() + iap.bufIdx) >= iap.bin.size)
        {
            return iap_write_bin_info();
        }

        if (iap.bufIdx >= (2 * KB))
        {
            return iap_write_bin(0);
        }
        else if (iap.mode == IAP_MODE_USB)
        {
            return IAP_WARN_BUF_NO_FULL;
        }
    }
    else if (iap.state == IAP_WAIT_BIN_INFO)
    {
        if (iap.bufIdx >= BIN_INFO_SIZE)
        {
            return iap_check_bin_info();
        }
        else if (iap.mode == IAP_MODE_USB)
        {
            return IAP_WARN_BUF_NO_FULL;
        }
    }

    return IAP_NO_ERR;
}

/**
 * 启动升级
 * @param mode          升级方式
 * @param binSize       bin文件大小
 * @return
 */
IapError iap_start_update(IapMode mode, uint32_t binSize)
{
    if (binSize > APP_MAX_SIZE)
        return iap_error(IAP_ERR_BIN_SIZE_OVERFLOW);

    iap.mode = mode;
    iap.bin.size = binSize;
    iap.flashAddr = APP_START_ADDRESS;
    iap.eraseAddr = iap_prepare_flash_area(APP_START_ADDRESS, binSize);
    iap.bufIdx = 0;
    iap.state = IAP_WAIT_BIN_INFO;


    return IAP_NO_ERR;
}

/**
 * 获取当前更新进度
 * @return
 */
uint8_t iap_update_percent(void)
{
    if (iap.state == IAP_IDLE)
        return 0;
    return ((iap_write_flash_size() * 100) / iap.bin.size);
}

/**
 * 获取iap升级模式
 * @return
 */
IapMode iap_mode(void)
{
    return iap.mode;
}
