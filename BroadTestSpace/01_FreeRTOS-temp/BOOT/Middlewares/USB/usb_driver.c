#include "usb_driver.h"
#include "cdc_acm_core.h"
#include <string.h>
#include <stdio.h>
#include "app_iap.h"
#include "valdefine.h"
#include "systick.h"
#include "device.h"
#include "addr.h"
#include "bsp_flash.h"
#include "utility.h"

#define USB_CDC_IS_READY        (((usb_cdc_handler *)usb_cdc_acm()->dev.class_data[CDC_COM_INTERFACE])->packet_sent)
#define USB_CDC_P_RXBUF         ((uint8_t *)(((usb_cdc_handler *) usb_cdc_acm()->dev.class_data[CDC_COM_INTERFACE])->data_rcv))
#define MAX_PACKET              (60)

static usb_core_driver cdcAcm;
static uint8_t usbConnect;

uint8_t usb_tx_buf[UPDATE_TXRX_SIZE];
uint32_t usb_tx_len = 0;

static void usb_pull_up_enable(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);

    gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_15);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15);

    gpio_bit_set(GPIOA, GPIO_PIN_15);
}

static void usb_gpio_config(void)
{
    rcu_periph_clock_enable(RCU_SYSCFG);
    rcu_periph_clock_enable(RCU_GPIOA);

    /* USBFS_DM(PA11) and USBFS_DP(PA12) GPIO pin configuration */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_11|GPIO_PIN_12);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_11|GPIO_PIN_12);
    gpio_af_set(GPIOA, GPIO_AF_10, GPIO_PIN_11|GPIO_PIN_12);
}

static void usb_rcu_config(void)
{
    rcu_pll48m_clock_config(RCU_PLL48MSRC_PLLQ);
    rcu_ck48m_clock_config(RCU_CK48MSRC_PLL48M);
    rcu_periph_clock_enable(RCU_USBFS);

//    rcu_pllsai_config(288, 6, 2);
//    RCU_CTL |= RCU_CTL_PLLSAIEN;
//    while(0U == (RCU_CTL & RCU_CTL_PLLSAISTB));
//    rcu_pll48m_clock_config(RCU_PLL48MSRC_PLLSAIP);
//    rcu_ck48m_clock_config(RCU_CK48MSRC_PLL48M);
//    rcu_periph_clock_enable(RCU_USBFS);
}

static uint16_t usb_cdc_read(void)
{
    usb_cdc_handler *cdc = (usb_cdc_handler *)usb_cdc_acm()->dev.class_data[CDC_COM_INTERFACE];

    if (1 == cdc->packet_receive)
    {
        cdc->packet_receive = 0;
        usbd_ep_recev(usb_cdc_acm(), CDC_DATA_OUT_EP, (uint8_t *)cdc->data_rcv, USB_CDC_DATA_PACKET_SIZE);
        return cdc->receive_length;
    }

    return 0;
}

void usb_init(void)
{
    usb_gpio_config();
    usb_rcu_config();
    usbd_init(usb_cdc_acm(), USB_CORE_ENUM_FS, &cdc_desc, &cdc_class);
    nvic_irq_enable(USBFS_IRQn, 2, 0);

    usb_pull_up_enable();
}

usb_core_driver *usb_cdc_acm(void)
{
    return &cdcAcm;
}

static int cdc_acm_ft232_send_packet(uint8_t *data, int data_len)
{
	int ret = -1;
	uint32_t timeout = 0;

	if ((data_len > MAX_PACKET) || (data_len <= 0))
	{
		return -1;
	}

	timeout = TaskDelay_GetTick();
	while (!cdc_acm_ft232_check_ready(usb_cdc_acm()))
	{
		if (USBD_CONFIGURED != usb_cdc_acm()->dev.cur_status)
		{
			return -1;
		}

		if ((TaskDelay_GetTick() - timeout) > 250)
		{
			return -1;
		}
	}

	ret = cdc_acm_ft232_send(usb_cdc_acm(), data, data_len);

	timeout = TaskDelay_GetTick();
	while (!cdc_acm_ft232_check_ready(usb_cdc_acm()))
	{
		if (USBD_CONFIGURED != usb_cdc_acm()->dev.cur_status)
		{
			return -1;
		}

		if ((TaskDelay_GetTick() - timeout) > 250)
		{
			return -1;
		}
	}

	return ret;
}

static int cdc_acm_ft232_sending(uint8_t *data, int data_len)
{
	int len = 0;
	int packet_len = 0;

	if (data_len <= 0)
		return -1;

	if (data_len <= MAX_PACKET)
	{
		return cdc_acm_ft232_send_packet(data, data_len);
	}
	else
	{
		while (len < data_len)
		{
			packet_len = USB_MIN(data_len - len, MAX_PACKET);
			packet_len = cdc_acm_ft232_send_packet(data + len, packet_len);

			if (packet_len > 0)
				len += packet_len;
			else
				return -1;
		}
		return len;
	}
}

int usb_cdc_write(uint8_t *buf, uint16_t len)
{
    return cdc_acm_ft232_sending(buf, len);
}

int usb_cdc_write_string(const char *s)
{
    return usb_cdc_write((uint8_t *)s, strlen(s));
}

static bool startBootUpgrad;
static bool firstPackage;
static uint16_t bootCrc;


static void rx_data_process(uint8_t *rx, uint16_t len)
{
    if (1 == iap_receive_available(IAP_MODE_USB))
    {
        IapError err = iap_write_handle(rx, len);
        if (err == IAP_NO_ERR)
        {
            usb_cdc_write_string(TRANS_ACK);
        }
        else if (err > IAP_WARN_BUF_NO_FULL)
        {
            usb_cdc_write_string(TRANS_FAIL);
            memcpy((uint8_t*)APP_UPDATE_FLAG, STR_UPDATE_FLAG, strlen(STR_UPDATE_FLAG));
            NVIC_SystemReset();
        }
        return;
    }

    if (0 == memcmp(rx, JCXX_IDENTITY, strlen(JCXX_IDENTITY)))
    {
        usbConnect = 1;
        usb_cdc_write_string(JCXX_BOOT);
    }
    else if (0 == memcmp(rx, JCXX_VERSION, strlen(JCXX_VERSION)))
    {
        usb_cdc_write_string(JCID_DEVICE_MID JCID_DEVICE_BOOT_VERS);
    }
    else if ((0 == memcmp(rx, "0x", strlen("0x"))) && (NULL != strstr((const char *)rx, JCID_DEVICE_BOOT_VERS)))
    {
        char strSize[9] = { 0 };
        memcpy(strSize, &rx[2], 8);
        char *stops;
        iap_start_update(IAP_MODE_USB, strtol(strSize, &stops, 16));
        usb_cdc_write_string(TRANS_UPDATE);
    }
}

static void board_write_to_usb(void)
{
    int writes = 0;

    if(usb_tx_len > 0)
    {
        writes = cdc_acm_ft232_sending(&usb_tx_buf[0], usb_tx_len);
        if (writes >= 0)
        {
            usb_tx_len -= writes;
            memcpy(&usb_tx_buf[0], &usb_tx_buf[writes], usb_tx_len);
        }
    }
}

void usb_cdc_process(void)
{	
    uint16_t len = usb_cdc_read();
    if (len)
    {
        rx_data_process(USB_CDC_P_RXBUF, len);
    }

    board_write_to_usb();

    static uint32_t heartbeat_tick;
    if (USBD_CONFIGURED == usb_cdc_acm()->dev.cur_status && USB_CDC_IS_READY)
	{
        if ((TaskDelay_GetTick() - heartbeat_tick) > 20)
        {
            cdc_acm_ft232_heartbeat(usb_cdc_acm());
            heartbeat_tick = TaskDelay_GetTick();
        }
    }
}

uint8_t usb_is_connected(void)
{
    return usbConnect;
}

void board_pack_buf_data(const char *buf, uint32_t len)
{
    memcpy(usb_tx_buf + usb_tx_len, buf, len);
    usb_tx_len += len;
}

bool usb_is_configured(void)
{
    return USBD_CONFIGURED == usb_cdc_acm()->dev.cur_status;
}


