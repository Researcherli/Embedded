#include "bsp_flash.h"
#include "gd32f4xx_fmc.h"

/* Base address of the Flash sectors */
#define ADDR_FLASH_SECTOR_0         ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1         ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2         ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3         ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4         ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5         ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6         ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7         ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_8         ((uint32_t)0x08080000) /* Base @ of Sector 8, 128 Kbytes */
#define ADDR_FLASH_SECTOR_9         ((uint32_t)0x080A0000) /* Base @ of Sector 9, 128 Kbytes */
#define ADDR_FLASH_SECTOR_10        ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_11        ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbytes */
#define ADDR_FLASH_SECTOR_12        ((uint32_t)0x08100000) /* Base @ of Sector 12, 16 Kbytes */
#define ADDR_FLASH_SECTOR_13        ((uint32_t)0x08104000) /* Base @ of Sector 13, 16 Kbytes */
#define ADDR_FLASH_SECTOR_14        ((uint32_t)0x08108000) /* Base @ of Sector 14, 16 Kbytes */
#define ADDR_FLASH_SECTOR_15        ((uint32_t)0x0810C000) /* Base @ of Sector 15, 16 Kbytes */
#define ADDR_FLASH_SECTOR_16        ((uint32_t)0x08110000) /* Base @ of Sector 16, 64 Kbytes */
#define ADDR_FLASH_SECTOR_17        ((uint32_t)0x08120000) /* Base @ of Sector 17, 128 Kbytes */
#define ADDR_FLASH_SECTOR_18        ((uint32_t)0x08140000) /* Base @ of Sector 18, 128 Kbytes */
#define ADDR_FLASH_SECTOR_19        ((uint32_t)0x08160000) /* Base @ of Sector 19, 128 Kbytes */
#define ADDR_FLASH_SECTOR_20        ((uint32_t)0x08180000) /* Base @ of Sector 20, 128 Kbytes */
#define ADDR_FLASH_SECTOR_21        ((uint32_t)0x081A0000) /* Base @ of Sector 21, 128 Kbytes */
#define ADDR_FLASH_SECTOR_22        ((uint32_t)0x081C0000) /* Base @ of Sector 22, 128 Kbytes */
#define ADDR_FLASH_SECTOR_23        ((uint32_t)0x081E0000) /* Base @ of Sector 23, 128 Kbytes */
#define ADDR_FLASH_SECTOR_24        ((uint32_t)0x08200000) /* Base @ of Sector 24, 256 Kbytes */
#define ADDR_FLASH_SECTOR_25        ((uint32_t)0x08240000) /* Base @ of Sector 25, 256 Kbytes */
#define ADDR_FLASH_SECTOR_26        ((uint32_t)0x08280000) /* Base @ of Sector 25, 256 Kbytes */
#define ADDR_FLASH_SECTOR_27        ((uint32_t)0x082C0000) /* Base @ of Sector 25, 256 Kbytes */

static uint32_t fmc_sector_num_get(uint32_t addr)
{
    uint32_t sector = 0;

    if ((addr < ADDR_FLASH_SECTOR_1) && (addr >= ADDR_FLASH_SECTOR_0))
    {
        sector = CTL_SECTOR_NUMBER_0;
    }
    else if ((addr < ADDR_FLASH_SECTOR_2) && (addr >= ADDR_FLASH_SECTOR_1))
    {
        sector = CTL_SECTOR_NUMBER_1;
    }
    else if ((addr < ADDR_FLASH_SECTOR_3) && (addr >= ADDR_FLASH_SECTOR_2))
    {
        sector = CTL_SECTOR_NUMBER_2;
    }
    else if ((addr < ADDR_FLASH_SECTOR_4) && (addr >= ADDR_FLASH_SECTOR_3))
    {
        sector = CTL_SECTOR_NUMBER_3;
    }
    else if ((addr < ADDR_FLASH_SECTOR_5) && (addr >= ADDR_FLASH_SECTOR_4))
    {
        sector = CTL_SECTOR_NUMBER_4;
    }
    else if ((addr < ADDR_FLASH_SECTOR_6) && (addr >= ADDR_FLASH_SECTOR_5))
    {
        sector = CTL_SECTOR_NUMBER_5;
    }
    else if ((addr < ADDR_FLASH_SECTOR_7) && (addr >= ADDR_FLASH_SECTOR_6))
    {
        sector = CTL_SECTOR_NUMBER_6;
    }
    else if ((addr < ADDR_FLASH_SECTOR_8) && (addr >= ADDR_FLASH_SECTOR_7))
    {
        sector = CTL_SECTOR_NUMBER_7;
    }
    else if ((addr < ADDR_FLASH_SECTOR_9) && (addr >= ADDR_FLASH_SECTOR_8))
    {
        sector = CTL_SECTOR_NUMBER_8;
    }
    else if ((addr < ADDR_FLASH_SECTOR_10) && (addr >= ADDR_FLASH_SECTOR_9))
    {
        sector = CTL_SECTOR_NUMBER_9;
    }
    else if ((addr < ADDR_FLASH_SECTOR_11) && (addr >= ADDR_FLASH_SECTOR_10))
    {
        sector = CTL_SECTOR_NUMBER_10;
    }
    else if ((addr < ADDR_FLASH_SECTOR_12) && (addr >= ADDR_FLASH_SECTOR_11))
    {
        sector = CTL_SECTOR_NUMBER_11;
    }
    else if ((addr < ADDR_FLASH_SECTOR_13) && (addr >= ADDR_FLASH_SECTOR_12))
    {
        sector = CTL_SECTOR_NUMBER_12;
    }
    else if ((addr < ADDR_FLASH_SECTOR_14) && (addr >= ADDR_FLASH_SECTOR_13))
    {
        sector = CTL_SECTOR_NUMBER_13;
    }
    else if ((addr < ADDR_FLASH_SECTOR_15) && (addr >= ADDR_FLASH_SECTOR_14))
    {
        sector = CTL_SECTOR_NUMBER_14;
    }
    else if ((addr < ADDR_FLASH_SECTOR_16) && (addr >= ADDR_FLASH_SECTOR_15))
    {
        sector = CTL_SECTOR_NUMBER_15;
    }
    else if ((addr < ADDR_FLASH_SECTOR_17) && (addr >= ADDR_FLASH_SECTOR_16))
    {
        sector = CTL_SECTOR_NUMBER_16;
    }
    else if ((addr < ADDR_FLASH_SECTOR_18) && (addr >= ADDR_FLASH_SECTOR_17))
    {
        sector = CTL_SECTOR_NUMBER_17;
    }
    else if ((addr < ADDR_FLASH_SECTOR_19) && (addr >= ADDR_FLASH_SECTOR_18))
    {
        sector = CTL_SECTOR_NUMBER_18;
    }
    else if ((addr < ADDR_FLASH_SECTOR_20) && (addr >= ADDR_FLASH_SECTOR_19))
    {
        sector = CTL_SECTOR_NUMBER_19;
    }
    else if ((addr < ADDR_FLASH_SECTOR_21) && (addr >= ADDR_FLASH_SECTOR_20))
    {
        sector = CTL_SECTOR_NUMBER_20;
    }
    else if ((addr < ADDR_FLASH_SECTOR_22) && (addr >= ADDR_FLASH_SECTOR_21))
    {
        sector = CTL_SECTOR_NUMBER_21;
    }
    else if ((addr < ADDR_FLASH_SECTOR_23) && (addr >= ADDR_FLASH_SECTOR_22))
    {
        sector = CTL_SECTOR_NUMBER_22;
    }
    else if ((addr < ADDR_FLASH_SECTOR_24) && (addr >= ADDR_FLASH_SECTOR_23))
    {
        sector = CTL_SECTOR_NUMBER_23;
    }
    else if ((addr < ADDR_FLASH_SECTOR_25) && (addr >= ADDR_FLASH_SECTOR_24))
    {
        sector = CTL_SECTOR_NUMBER_24;
    }
    else if ((addr < ADDR_FLASH_SECTOR_26) && (addr >= ADDR_FLASH_SECTOR_25))
    {
        sector = CTL_SECTOR_NUMBER_25;
    }
    else if ((addr < ADDR_FLASH_SECTOR_27) && (addr >= ADDR_FLASH_SECTOR_26))
    {
        sector = CTL_SECTOR_NUMBER_26;
    }
    else
    {
        sector = CTL_SECTOR_NUMBER_27;
    }

    return sector;
}

void flash_erase_sector(uint32_t addr)
{
    /* get information about the sector in which the specified address is located */
    uint32_t sectorNum = fmc_sector_num_get(addr);

    /* unlock the flash program erase controller */
    fmc_unlock();
    /* clear pending flags */
    fmc_flag_clear(FMC_FLAG_END|FMC_FLAG_OPERR|FMC_FLAG_WPERR|FMC_FLAG_PGMERR|FMC_FLAG_PGSERR);
    /* wait the erase operation complete*/
    if (FMC_READY != fmc_sector_erase(sectorNum))
    {
        while (1);
    }
    /* lock the flash program erase controller */
    fmc_lock();
}

void flash_write_data(uint32_t addr, uint8_t *buf, uint32_t len)
{
    /* unlock the flash program erase controller */
    fmc_unlock();
    /* clear pending flags */
    fmc_flag_clear(FMC_FLAG_END|FMC_FLAG_OPERR|FMC_FLAG_WPERR|FMC_FLAG_PGMERR|FMC_FLAG_PGSERR);

    /* program flash */
    for (uint32_t i = 0; i < len; i += 4)
    {
        uint32_t data = *((uint32_t *)(&buf[i]));
        __IO fmc_state_enum tmp = fmc_word_program(addr + i, data);
        if (tmp > FMC_READY)
        {
            while (1);
        }
        fmc_flag_clear(FMC_FLAG_END|FMC_FLAG_OPERR|FMC_FLAG_WPERR|FMC_FLAG_PGMERR|FMC_FLAG_PGSERR);
    }

    fmc_flag_clear(FMC_FLAG_END|FMC_FLAG_OPERR|FMC_FLAG_WPERR|FMC_FLAG_PGMERR|FMC_FLAG_PGSERR);

    /* lock the main FMC after the program operation */
    fmc_lock();
}

uint32_t flash_sector_size_get(uint32_t addr)
{
    uint32_t sector = 0;

    if ((addr < ADDR_FLASH_SECTOR_1) && (addr >= ADDR_FLASH_SECTOR_0))
    {
        sector = 0x4000;
    }
    else if ((addr < ADDR_FLASH_SECTOR_2) && (addr >= ADDR_FLASH_SECTOR_1))
    {
        sector = 0x4000;
    }
    else if ((addr < ADDR_FLASH_SECTOR_3) && (addr >= ADDR_FLASH_SECTOR_2))
    {
        sector = 0x4000;
    }
    else if ((addr < ADDR_FLASH_SECTOR_4) && (addr >= ADDR_FLASH_SECTOR_3))
    {
        sector = 0x4000;
    }
    else if ((addr < ADDR_FLASH_SECTOR_5) && (addr >= ADDR_FLASH_SECTOR_4))
    {
        sector = 0x10000;
    }
    else if ((addr < ADDR_FLASH_SECTOR_6) && (addr >= ADDR_FLASH_SECTOR_5))
    {
        sector = 0x20000;
    }
    else if ((addr < ADDR_FLASH_SECTOR_7) && (addr >= ADDR_FLASH_SECTOR_6))
    {
        sector = 0x20000;
    }
    else if ((addr < ADDR_FLASH_SECTOR_8) && (addr >= ADDR_FLASH_SECTOR_7))
    {
        sector = 0x20000;
    }
    else if ((addr < ADDR_FLASH_SECTOR_9) && (addr >= ADDR_FLASH_SECTOR_8))
    {
        sector = 0x20000;
    }
    else if ((addr < ADDR_FLASH_SECTOR_10) && (addr >= ADDR_FLASH_SECTOR_9))
    {
        sector = 0x20000;
    }
    else if ((addr < ADDR_FLASH_SECTOR_11) && (addr >= ADDR_FLASH_SECTOR_10))
    {
        sector = 0x20000;
    }
    else if ((addr < ADDR_FLASH_SECTOR_12) && (addr >= ADDR_FLASH_SECTOR_11))
    {
        sector = 0x20000;
    }
    else if ((addr < ADDR_FLASH_SECTOR_13) && (addr >= ADDR_FLASH_SECTOR_12))
    {
        sector = 0x4000;
    }
    else if ((addr < ADDR_FLASH_SECTOR_14) && (addr >= ADDR_FLASH_SECTOR_13))
    {
        sector = 0x4000;
    }
    else if ((addr < ADDR_FLASH_SECTOR_15) && (addr >= ADDR_FLASH_SECTOR_14))
    {
        sector = 0x4000;
    }
    else if ((addr < ADDR_FLASH_SECTOR_16) && (addr >= ADDR_FLASH_SECTOR_15))
    {
        sector = 0x4000;
    }
    else if ((addr < ADDR_FLASH_SECTOR_17) && (addr >= ADDR_FLASH_SECTOR_16))
    {
        sector = 0x10000;
    }
    else if ((addr < ADDR_FLASH_SECTOR_18) && (addr >= ADDR_FLASH_SECTOR_17))
    {
        sector = 0x20000;
    }
    else if ((addr < ADDR_FLASH_SECTOR_19) && (addr >= ADDR_FLASH_SECTOR_18))
    {
        sector = 0x20000;
    }
    else if ((addr < ADDR_FLASH_SECTOR_20) && (addr >= ADDR_FLASH_SECTOR_19))
    {
        sector = 0x20000;
    }
    else if ((addr < ADDR_FLASH_SECTOR_21) && (addr >= ADDR_FLASH_SECTOR_20))
    {
        sector = 0x20000;
    }
    else if ((addr < ADDR_FLASH_SECTOR_22) && (addr >= ADDR_FLASH_SECTOR_21))
    {
        sector = 0x20000;
    }
    else if ((addr < ADDR_FLASH_SECTOR_23) && (addr >= ADDR_FLASH_SECTOR_22))
    {
        sector = 0x20000;
    }
    else if ((addr < ADDR_FLASH_SECTOR_24) && (addr >= ADDR_FLASH_SECTOR_23))
    {
        sector = 0x20000;
    }
    else if ((addr < ADDR_FLASH_SECTOR_25) && (addr >= ADDR_FLASH_SECTOR_24))
    {
        sector = 0x40000;
    }
    else if ((addr < ADDR_FLASH_SECTOR_26) && (addr >= ADDR_FLASH_SECTOR_25))
    {
        sector = 0x40000;
    }
    else if ((addr < ADDR_FLASH_SECTOR_27) && (addr >= ADDR_FLASH_SECTOR_26))
    {
        sector = 0x40000;
    }
    else
    {
        sector = 0x40000;
    }

    return sector;
}
