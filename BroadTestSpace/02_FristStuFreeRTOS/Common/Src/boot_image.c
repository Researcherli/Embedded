#include "boot_image.h"
#include "boot_crc32.h"

static uint32_t app_base(boot_slot_enum slot)
{
    return (slot == SLOT_APP_A) ? PART_APP_A_ADDR : PART_APP_B_ADDR;
}

static uint32_t app_size(boot_slot_enum slot)
{
    return (slot == SLOT_APP_A) ? PART_APP_A_SIZE : PART_APP_B_SIZE;
}

boot_image_status_t boot_image_verify(const boot_param_t *param, boot_slot_enum slot)
{
    if (param == NULL) {
        return IMAGE_EMPTY;
    }
    if ((uint32_t)slot > (uint32_t)SLOT_APP_B) {
        return IMAGE_SIZE_INVALID;
    }

    uint32_t size   = PARAM_SLOT_INFO(param, slot).fw_size;
    uint32_t expect = PARAM_SLOT_INFO(param, slot).fw_crc32;

    if (size == 0U) {
        return IMAGE_EMPTY;         /* 该槽还没写过固件 */
    }
    if (size > app_size(slot)) {
        return IMAGE_SIZE_INVALID;  /* 记录的长度超出分区，防止越界读 */
    }

    /* 分块累加：768KB 镜像无法一次性缓冲进 RAM */
    uint32_t addr   = app_base(slot);
    uint32_t remain = size;
    uint32_t crc    = 0xFFFFFFFFU;

    while (remain > 0U) {
        uint32_t chunk = (remain > BOOT_IMAGE_CHUNK_SIZE) ? BOOT_IMAGE_CHUNK_SIZE : remain;
        crc = boot_crc32_update(crc, (const uint8_t *)(uintptr_t)addr, chunk);
        addr   += chunk;
        remain -= chunk;
    }

    return ((~crc) == expect) ? IMAGE_OK : IMAGE_CRC_MISMATCH;
}

const char *boot_image_status_str(boot_image_status_t status)
{
    switch (status) {
    case IMAGE_OK:              return "OK";
    case IMAGE_EMPTY:           return "EMPTY";
    case IMAGE_SIZE_INVALID:    return "SIZE_INVALID";
    case IMAGE_CRC_MISMATCH:    return "CRC_MISMATCH";
    default:                    return "UNKNOWN";
    }
}
