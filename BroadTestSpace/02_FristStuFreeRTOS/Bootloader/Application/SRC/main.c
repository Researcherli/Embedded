#include "gd32f4xx.h"
#include "systick.h"
#include "boot_partition.h"

#include "usb_driver.h"
#include "bsp_flash.h"

STATIC_ASSERT_PARTITION();

int main(void)
{
	systick_init();
	usb_init();

	while (1)
	{
		usb_cdc_process();
	}
}
