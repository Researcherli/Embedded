#ifndef BSP_FLASH_H
#define BSP_FLASH_H

#include <stdint.h>
#include <stdbool.h>

bool flash_erase_sector_by_addr(uint32_t addr);
bool flash_write_data(uint32_t addr, const uint8_t *buf, uint32_t len);
uint32_t flash_sector_size_get(uint32_t addr);

#endif // BSP_FLASH_H