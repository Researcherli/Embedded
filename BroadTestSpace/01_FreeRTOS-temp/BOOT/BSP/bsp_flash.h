#ifndef BSP_FLASH_H
#define BSP_FLASH_H

#include <stdint.h>

void flash_erase_sector(uint32_t addr);
void flash_write_data(uint32_t addr, uint8_t *buf, uint32_t len);
uint32_t flash_sector_size_get(uint32_t addr);

#endif // BSP_FLASH_H
