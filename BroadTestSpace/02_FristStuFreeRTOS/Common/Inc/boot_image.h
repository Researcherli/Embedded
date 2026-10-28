#ifndef BOOT_IMAGE_H
#define BOOT_IMAGE_H

#include <stdint.h>
#include "boot_param.h"

/* 校验时每次从 Flash 读取的块大小（字节），需为 4 的整数倍 */
#define BOOT_IMAGE_CHUNK_SIZE   1024U

/* 镜像校验结果 */
typedef enum {
    IMAGE_OK = 0,           /* 校验通过 */
    IMAGE_EMPTY,            /* 该槽尚未写入固件（fw_size 为 0） */
    IMAGE_SIZE_INVALID,     /* fw_size 超出该槽分区容量，或槽号非法 */
    IMAGE_CRC_MISMATCH      /* 内容与记录的 CRC32 不符 */
} boot_image_status_t;

/**
 * @brief 校验指定槽内的固件镜像
 *
 * 以 boot_param 中记录的 fw_size / fw_crc32 为准，按 BOOT_IMAGE_CHUNK_SIZE 分块
 * 读取 Flash 并累加 CRC，不占用大块 RAM（768KB 镜像无法一次性缓冲）。
 *
 * @param param 已加载的参数
 * @param slot  待校验的槽（SLOT_APP_A / SLOT_APP_B）
 * @return 校验结果，IMAGE_OK 表示通过
 */
boot_image_status_t boot_image_verify(const boot_param_t *param, boot_slot_enum slot);

/**
 * @brief 取校验结果的文字说明，便于日志与调试器观察
 */
const char *boot_image_status_str(boot_image_status_t status);

#endif /* BOOT_IMAGE_H */
