#ifndef UTILITY_H
#define UTILITY_H

#include <stdint.h>

uint8_t crc8(uint8_t *data, uint32_t len);
uint16_t crc16(uint8_t *data, uint32_t len);
char *mcu_unique_id(void);
int string_search(char *s, char ch);

#endif
