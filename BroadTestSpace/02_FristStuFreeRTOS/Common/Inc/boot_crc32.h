#ifndef BOOT_CRC32_H
#define BOOT_CRC32_H

#include <stdint.h>

/**
 * @brief CRC32 增量计算（反射式，多项式 0xEDB88320），结果与 zlib crc32 一致
 *
 * 分段累加用法：
 *     uint32_t crc = 0xFFFFFFFFU;
 *     crc = boot_crc32_update(crc, chunk1, len1);
 *     crc = boot_crc32_update(crc, chunk2, len2);
 *     uint32_t final = ~crc;
 *
 * @param crc    上一次的返回值；首次调用传 0xFFFFFFFFU
 * @param data   本段数据起始地址
 * @param length 本段字节数
 * @return 中间结果；全部喂完后取反才得到最终 CRC
 */
uint32_t boot_crc32_update(uint32_t crc, const uint8_t *data, uint32_t length);

/**
 * @brief 一次性计算一段数据的 CRC32
 */
uint32_t boot_crc32_calc(const uint8_t *data, uint32_t length);

#endif /* BOOT_CRC32_H */
