#define  OS_GLOBALS
#include "gd32f4xx.h"
#include "systick.h"
#include "app_iap.h"

int main(void)
{
    systick_init();
    iap_start();
}
