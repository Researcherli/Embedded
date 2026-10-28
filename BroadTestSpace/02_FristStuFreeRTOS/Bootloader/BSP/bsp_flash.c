#include "bsp_flash.h"
#include "gd32f4xx_fmc.h"
#include <string.h>

typedef struct {
    uint32_t start_addr;
    uint32_t size;
    uint32_t ctl_sector;
} flash_sector_desc_t;

/* 查表法替代冗长的 if-else */
static const flash_sector_desc_t g_flash_sectors[] = {
    {0x08000000, 0x004000, CTL_SECTOR_NUMBER_0},	// 16KB
    {0x08004000, 0x004000, CTL_SECTOR_NUMBER_1},	// 16KB
    {0x08008000, 0x004000, CTL_SECTOR_NUMBER_2},	// 16KB
    {0x0800C000, 0x004000, CTL_SECTOR_NUMBER_3},	// 16KB
    {0x08010000, 0x010000, CTL_SECTOR_NUMBER_4},	// 64KB
    {0x08020000, 0x020000, CTL_SECTOR_NUMBER_5},	// 128KB
    {0x08040000, 0x020000, CTL_SECTOR_NUMBER_6},	// 128KB
    {0x08060000, 0x020000, CTL_SECTOR_NUMBER_7},	// 128KB
    {0x08080000, 0x020000, CTL_SECTOR_NUMBER_8},	// 128KB
    {0x080A0000, 0x020000, CTL_SECTOR_NUMBER_9},	// 128KB
    {0x080C0000, 0x020000, CTL_SECTOR_NUMBER_10},	// 128KB
    {0x080E0000, 0x020000, CTL_SECTOR_NUMBER_11},	// 128KB
    /* Bank 1 扇区 */
    {0x08100000, 0x004000, CTL_SECTOR_NUMBER_12},	// 16KB
    {0x08104000, 0x004000, CTL_SECTOR_NUMBER_13},	// 16KB
    {0x08108000, 0x004000, CTL_SECTOR_NUMBER_14},	// 16KB
    {0x0810C000, 0x004000, CTL_SECTOR_NUMBER_15},	// 16KB
    {0x08110000, 0x010000, CTL_SECTOR_NUMBER_16},	// 64KB
    {0x08120000, 0x020000, CTL_SECTOR_NUMBER_17},	// 128KB
    {0x08140000, 0x020000, CTL_SECTOR_NUMBER_18},	// 128KB
    {0x08160000, 0x020000, CTL_SECTOR_NUMBER_19},	// 128KB
    {0x08180000, 0x020000, CTL_SECTOR_NUMBER_20},	// 128KB
    {0x081A0000, 0x020000, CTL_SECTOR_NUMBER_21},	// 128KB
    {0x081C0000, 0x020000, CTL_SECTOR_NUMBER_22},	// 128KB
    {0x081E0000, 0x020000, CTL_SECTOR_NUMBER_23},	// 128KB
};
#define SECTOR_TOTAL_CNT (sizeof(g_flash_sectors) / sizeof(g_flash_sectors[0]))

/* 获取扇区索引，非法返回 -1 */
static int32_t get_sector_index(uint32_t addr)
{
    for (uint32_t i = 0; i < SECTOR_TOTAL_CNT; i++) {
        if ((addr >= g_flash_sectors[i].start_addr) &&
            (addr < (g_flash_sectors[i].start_addr + g_flash_sectors[i].size))) {
            return (int32_t)i;
        }
    }
    return -1; // 越界非法
}

uint32_t flash_sector_size_get(uint32_t addr)
{
    int32_t idx = get_sector_index(addr);
    return (idx >= 0) ? g_flash_sectors[idx].size : 0;
}

bool flash_erase_sector_by_addr(uint32_t addr)
{
    int32_t idx = get_sector_index(addr);
    if (idx < 0) return false;

    fmc_unlock();
    fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_OPERR | FMC_FLAG_WPERR | FMC_FLAG_PGMERR | FMC_FLAG_PGSERR);

    fmc_state_enum state = fmc_sector_erase(g_flash_sectors[idx].ctl_sector);

    fmc_lock();
    return (state == FMC_READY);
}

bool flash_write_data(uint32_t addr, const uint8_t *buf, uint32_t len)
{
    if ((addr % 4) != 0 || buf == NULL || len == 0) return false;

    fmc_unlock();
    fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_OPERR | FMC_FLAG_WPERR | FMC_FLAG_PGMERR | FMC_FLAG_PGSERR);

    uint32_t words = len / 4;
    uint32_t remain_bytes = len % 4;

    /* 1. 先写 4 字节整对齐部分（通过 memcpy 避免非对齐指针引发 HardFault） */
    for (uint32_t i = 0; i < words; i++) {
        uint32_t temp_val;
        memcpy(&temp_val, &buf[i * 4], 4);

        if (fmc_word_program(addr + (i * 4), temp_val) != FMC_READY) {
            fmc_lock();
            return false;
        }
        fmc_flag_clear(FMC_FLAG_END);
    }

    /* 2. 处理不满 4 字节的末尾残余（补 0xFF，防止越界读并保留未编程位） */
    if (remain_bytes > 0) {
        uint32_t temp_val = 0xFFFFFFFF;
        memcpy(&temp_val, &buf[words * 4], remain_bytes);

        if (fmc_word_program(addr + (words * 4), temp_val) != FMC_READY) {
            fmc_lock();
            return false;
        }
        fmc_flag_clear(FMC_FLAG_END);
    }

    fmc_lock();
    return true;
}