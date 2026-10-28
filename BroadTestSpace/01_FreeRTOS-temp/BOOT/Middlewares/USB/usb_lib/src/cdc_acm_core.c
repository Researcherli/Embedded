/*!
    \file    cdc_acm_core.c
    \brief   CDC ACM driver

    \version 2020-08-01, V3.0.0, firmware for GD32F4xx
    \version 2020-12-07, V3.0.1, firmware for GD32F4xx
*/

/*
    Copyright (c) 2020, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#include <string.h>
#include "cdc_acm_core.h"

#define USBD_VID                          0x0403U//0x28E9U
#define USBD_PID                          0x6001U//0x018AU
typedef struct
{
    uint32_t    baud;
    uint32_t    data;
    uint32_t    stop;
    uint32_t    parity;
    
}uart_para_t;

/* note:it should use the C99 standard when compiling the below codes */
/* USB standard device descriptor */
__ALIGN_BEGIN const usb_desc_dev cdc_dev_desc __ALIGN_END =
{
    .header = 
     {
         .bLength          = USB_DEV_DESC_LEN, 
         .bDescriptorType  = USB_DESCTYPE_DEV,
     },
    .bcdUSB                = 0x0110U,
    .bDeviceClass          = USB_CLASS_CDC,
    .bDeviceSubClass       = 0x00U,
    .bDeviceProtocol       = 0x00U,
    .bMaxPacketSize0       = USB_FS_EP0_MAX_LEN,
    .idVendor              = USBD_VID,
    .idProduct             = USBD_PID,
    .bcdDevice             = 0x0400U,
    .iManufacturer         = STR_IDX_MFC,
    .iProduct              = STR_IDX_PRODUCT,
    .iSerialNumber         = STR_IDX_SERIAL,
    .bNumberConfigurations = 0x01,//USBD_CFG_MAX_NUM,
};

/* USB device configuration descriptor */
__ALIGN_BEGIN const usb_cdc_desc_config_set cdc_config_desc __ALIGN_END = 
{
    .config = 
    {
        .header = 
         {
             .bLength         = sizeof(usb_desc_config), 
             .bDescriptorType = USB_DESCTYPE_CONFIG,
         },
        .wTotalLength         = USB_CDC_ACM_CONFIG_DESC_SIZE,
        .bNumInterfaces       = 0x01U,
        .bConfigurationValue  = 0x01U,
        .iConfiguration       = 0x00U,
        .bmAttributes         = 0xA0U,
        .bMaxPower            = 0x32U
    },
/*
    .cmd_itf = 
    {
        .header = 
         {
             .bLength         = sizeof(usb_desc_itf), 
             .bDescriptorType = USB_DESCTYPE_ITF 
         },
        .bInterfaceNumber     = 0x00U,
        .bAlternateSetting    = 0x00U,
        .bNumEndpoints        = 0x02U,
        .bInterfaceClass      = 0xDCU,//USB_CLASS_CDC,
        .bInterfaceSubClass   = 0x00U,//USB_CDC_SUBCLASS_ACM,
        .bInterfaceProtocol   = 0x00U,//USB_CDC_PROTOCOL_AT,
        .iInterface           = 0x00U
    },

    .cdc_header = 
    {
        .header =
         {
            .bLength         = sizeof(usb_desc_header_func), 
            .bDescriptorType = USB_DESCTYPE_CS_INTERFACE
         },
        .bDescriptorSubtype  = 0x00U,
        .bcdCDC              = 0x0110U
    },

    .cdc_call_managment = 
    {
        .header = 
         {
            .bLength         = sizeof(usb_desc_call_managment_func), 
            .bDescriptorType = USB_DESCTYPE_CS_INTERFACE
         },
        .bDescriptorSubtype  = 0x01U,
        .bmCapabilities      = 0x00U,
        .bDataInterface      = 0x01U
    },

    .cdc_acm = 
    {
        .header = 
         {
            .bLength         = sizeof(usb_desc_acm_func), 
            .bDescriptorType = USB_DESCTYPE_CS_INTERFACE
         },
        .bDescriptorSubtype  = 0x02U,
        .bmCapabilities      = 0x02U,
    },

    .cdc_union = 
    {
        .header = 
         {
            .bLength         = sizeof(usb_desc_union_func), 
            .bDescriptorType = USB_DESCTYPE_CS_INTERFACE
         },
        .bDescriptorSubtype  = 0x06U,
        .bMasterInterface    = 0x00U,
        .bSlaveInterface0    = 0x01U,
    },

    .cdc_cmd_endpoint = 
    {
        .header = 
         {
            .bLength         = sizeof(usb_desc_ep), 
            .bDescriptorType = USB_DESCTYPE_EP,
         },
        .bEndpointAddress    = CDC_CMD_EP,
        .bmAttributes        = USB_EP_ATTR_INT,
        .wMaxPacketSize      = USB_CDC_CMD_PACKET_SIZE,
        .bInterval           = 0x0AU
    },
*/
    .cdc_data_interface = 
    {
        .header = 
         {
            .bLength         = sizeof(usb_desc_itf), 
            .bDescriptorType = USB_DESCTYPE_ITF,
         },
        .bInterfaceNumber    = 0x00U,
        .bAlternateSetting   = 0x00U,
        .bNumEndpoints       = 0x02U,
        .bInterfaceClass     = 0xFFU,
        .bInterfaceSubClass  = 0xFFU,
        .bInterfaceProtocol  = 0xFFU,
        .iInterface          = 0x02U
    },

    .cdc_out_endpoint = 
    {
        .header = 
         {
             .bLength         = sizeof(usb_desc_ep), 
             .bDescriptorType = USB_DESCTYPE_EP, 
         },
        .bEndpointAddress     = CDC_DATA_OUT_EP,
        .bmAttributes         = USB_EP_ATTR_BULK,
        .wMaxPacketSize       = USB_CDC_DATA_PACKET_SIZE,
        .bInterval            = 0x00U
    },

    .cdc_in_endpoint = 
    {
        .header = 
         {
             .bLength         = sizeof(usb_desc_ep), 
             .bDescriptorType = USB_DESCTYPE_EP 
         },
        .bEndpointAddress     = CDC_DATA_IN_EP,
        .bmAttributes         = USB_EP_ATTR_BULK,
        .wMaxPacketSize       = USB_CDC_DATA_PACKET_SIZE,
        .bInterval            = 0x00U
    }
};

/* USB language ID Descriptor */
static __ALIGN_BEGIN const usb_desc_LANGID usbd_language_id_desc __ALIGN_END = 
{
    .header = 
     {
         .bLength         = sizeof(usb_desc_LANGID), 
         .bDescriptorType = USB_DESCTYPE_STR,
     },
    .wLANGID              = ENG_LANGID
};

/* USB manufacture string */
static __ALIGN_BEGIN usb_desc_str manufacturer_string __ALIGN_END = 
{
    .header = 
     {
         .bLength         = USB_STRING_LEN(1U), 
         .bDescriptorType = USB_DESCTYPE_STR,
     },
};

/* USB product string */
static __ALIGN_BEGIN usb_desc_str product_string __ALIGN_END = 
{
    .header = 
     {
         .bLength         = USB_STRING_LEN(1U), 
         .bDescriptorType = USB_DESCTYPE_STR,
     },
};

/* USBD serial string */
static __ALIGN_BEGIN usb_desc_str serial_string __ALIGN_END = 
{
    .header = 
     {
         .bLength         = USB_STRING_LEN(1U), 
         .bDescriptorType = USB_DESCTYPE_STR,
     },
};

/* USB string descriptor set */
void *const usbd_cdc_strings[] = 
{
    [STR_IDX_LANGID]  = (uint8_t *)&usbd_language_id_desc,
    [STR_IDX_MFC]     = (uint8_t *)&manufacturer_string,
    [STR_IDX_PRODUCT] = (uint8_t *)&product_string,
    [STR_IDX_SERIAL]  = (uint8_t *)&serial_string
};

usb_desc cdc_desc = 
{
    .dev_desc    = (uint8_t *)&cdc_dev_desc,
    .config_desc = (uint8_t *)&cdc_config_desc,
    .strings     = usbd_cdc_strings,
};

/* local function prototypes ('static') */
static uint8_t cdc_acm_init   (usb_dev *udev, uint8_t config_index);
static uint8_t cdc_acm_deinit (usb_dev *udev, uint8_t config_index);
static uint8_t cdc_acm_req    (usb_dev *udev, usb_req *req);
static uint8_t cdc_ctlx_out   (usb_dev *udev);
static uint8_t cdc_acm_in     (usb_dev *udev, uint8_t ep_num);
static uint8_t cdc_acm_out    (usb_dev *udev, uint8_t ep_num);

/* USB CDC device class callbacks structure */
usb_class_core cdc_class =
{
    .command   = NO_CMD,
    .alter_set = 0U,

    .init      = cdc_acm_init,
    .deinit    = cdc_acm_deinit,

    .req_proc  = cdc_acm_req,
    .ctlx_out  = cdc_ctlx_out,

    .data_in   = cdc_acm_in,
    .data_out  = cdc_acm_out
};

/*!
    \brief      send CDC ACM data
    \param[in]  udev: pointer to USB device instance
    \param[out] none
    \retval     USB device operation status
*/
void cdc_acm_data_send (usb_dev *udev)
{
    usb_cdc_handler *cdc = (usb_cdc_handler *)udev->dev.class_data[CDC_COM_INTERFACE];

    if (0U != cdc->receive_length) {
        cdc->packet_sent = 0U;

        usbd_ep_send (udev, CDC_DATA_IN_EP, (uint8_t*)(cdc->data_snd), cdc->receive_length);

        cdc->receive_length = 0U;
    }
}

/*!
    \brief      receive CDC ACM data
    \param[in]  udev: pointer to USB device instance
    \param[out] none
    \retval     USB device operation status
*/
void cdc_acm_data_receive (usb_dev *udev)
{
    usb_cdc_handler *cdc = (usb_cdc_handler *)udev->dev.class_data[CDC_COM_INTERFACE];

    cdc->packet_receive = 0U;
    //cdc->packet_sent = 0U;

    usbd_ep_recev(udev, CDC_DATA_OUT_EP, (uint8_t*)(cdc->data_rcv), USB_CDC_DATA_PACKET_SIZE);
}

/*----------------------------------------------------------------------------*/
int cdc_acm_ft232_send (usb_dev *udev, uint8_t*data, uint32_t data_len)
{
    usb_cdc_handler *cdc = (usb_cdc_handler *)udev->dev.class_data[CDC_COM_INTERFACE];

    if(data_len>(USB_CDC_RX_LEN-2))
        return -1;
 
    if (1U == cdc->packet_sent) {
      
        cdc->packet_sent = 0U;
        cdc->data_snd[0] = 0x01;
        cdc->data_snd[1] = 0x01;
      
        if(data_len)
            memcpy(cdc->data_snd+2,data,data_len);
        
        usbd_ep_send(udev, CDC_DATA_IN_EP, (uint8_t*)(cdc->data_snd), (uint16_t)(data_len+2));
        
        return data_len;
    }
    
    return 0;
}

/*----------------------------------------------------------------------------*/
int cdc_acm_ft232_heartbeat (usb_dev *udev)
{
    usb_cdc_handler *cdc = (usb_cdc_handler *)udev->dev.class_data[CDC_COM_INTERFACE];

    if (1U == cdc->packet_sent) {
        cdc->packet_sent = 0U;
        cdc->data_snd[0] = 0x01;
        cdc->data_snd[1] = 0x60;

        usbd_ep_send(udev, CDC_DATA_IN_EP, (uint8_t*)(cdc->data_snd), (uint16_t)(2)); 
    }
    
    return 0;
}

/*----------------------------------------------------------------------------*/
int cdc_acm_ft232_recv(usb_dev *udev, uint8_t*data, uint32_t data_len)
{
    usb_cdc_handler *cdc = (usb_cdc_handler *)udev->dev.class_data[CDC_COM_INTERFACE];
    if(cdc->packet_receive == 1U)
    {
        uint16_t len = USB_MIN(data_len, cdc->receive_length);
        memcpy(data,cdc->data_rcv,len);
        cdc_acm_data_receive(udev);
        return len;

    }
    return 0;
}

/*----------------------------------------------------------------------------*/
/* 1 if cdc is ready, 0 otherwise */
uint8_t cdc_acm_ft232_check_ready(usb_dev *udev)
{
    if (udev->dev.class_data[CDC_COM_INTERFACE] != NULL) {
        usb_cdc_handler *cdc = (usb_cdc_handler *)udev->dev.class_data[CDC_COM_INTERFACE];

        if ((1U == cdc->packet_sent)) {
            return 1U;
        }
    }

    return 0U;
}

/*----------------------------------------------------------------------------*/
/* 1 if config is ready, 0 otherwise */
uint8_t cdc_acm_ft232_check_configured(usb_dev *udev)
{
    if(USBD_CONFIGURED == udev->dev.cur_status)
    {
        return 1;
    }
    
    return 0;
}

/*!
    \brief      initialize the CDC ACM device
    \param[in]  udev: pointer to USB device instance
    \param[in]  config_index: configuration index
    \param[out] none
    \retval     USB device operation status
*/
static uint8_t cdc_acm_init (usb_dev *udev, uint8_t config_index)
{
    static __ALIGN_BEGIN usb_cdc_handler cdc_handler __ALIGN_END;

    /* initialize the data TX endpoint */
    usbd_ep_setup (udev, &(cdc_config_desc.cdc_in_endpoint));

    /* initialize the data RX endpoint */
    usbd_ep_setup (udev, &(cdc_config_desc.cdc_out_endpoint));

    /* initialize the command TX endpoint */
    //usbd_ep_setup (udev, &(cdc_config_desc.cdc_cmd_endpoint));

    /* initialize CDC handler structure */
    cdc_handler.packet_receive = 1U;
    cdc_handler.packet_sent = 1U;
    cdc_handler.receive_length = 0U;

    cdc_handler.line_coding = (acm_line){
        .dwDTERate   = 115200,
        .bCharFormat = 0,
        .bParityType = 0,
        .bDataBits   = 0x08
    };

    udev->dev.class_data[CDC_COM_INTERFACE] = (void *)&cdc_handler;

    return USBD_OK;
}

/*!
    \brief      de-initialize the CDC ACM device
    \param[in]  udev: pointer to USB device instance
    \param[in]  config_index: configuration index
    \param[out] none
    \retval     USB device operation status
*/
static uint8_t cdc_acm_deinit (usb_dev *udev, uint8_t config_index)
{
    /* deinitialize the data TX/RX endpoint */
    usbd_ep_clear (udev, CDC_DATA_IN_EP);
    usbd_ep_clear (udev, CDC_DATA_OUT_EP);

    /* deinitialize the command TX endpoint */
    //usbd_ep_clear (udev, CDC_CMD_EP);

    return USBD_OK;
}

/*!
    \brief      handle the CDC ACM class-specific requests
    \param[in]  udev: pointer to USB device instance
    \param[in]  req: device class-specific request
    \param[out] none
    \retval     USB device operation status
*/
static uart_para_t uart_para = {0x001A, 0x08, 0x00, 0x00};
static uint8_t cdc_acm_req (usb_dev *udev, usb_req *req)
{
//	  usb_cdc_handler *cdc = (usb_cdc_handler *)udev->dev.class_data[CDC_COM_INTERFACE];
//	  usb_transc *transc = &udev->dev.transc_in[0];
	
	  if((req->bmRequestType & USB_REQTYPE_MASK) == USB_REQTYPE_VENDOR)
    {
        
        uint16_t wValue = req->wValue;
        
        if (req->bRequest <=0x04)
        {
            if(0x03 == req->bRequest)
            {
                uart_para.baud = wValue;
            }
            if(0x04 == req->bRequest)
            {
                uart_para.data   = wValue & 0xFF00;
                uart_para.stop   = wValue & 0x00F0;
                uart_para.parity = wValue & 0x000F;
            }
            if((0x01 == req->bRequest) && (0x0200 == wValue))
            {
//                board_set_online_stat(0);
            }
            //ft232_change_uart_para(uart_para.baud, uart_para.data, uart_para.stop, uart_para.parity);
        }
  /*      else if (req->bRequest ==0x05)
        {
            cdc->data_cmd[0] =0x01;
            cdc->data_cmd[1] =0x60;
            transc->xfer_buf = cdc->data_cmd;
            transc->remain_len = 2U;
        }
        else if(req->bRequest==0x90)   
        {
            cdc->data_cmd[0] =0xff;
            cdc->data_cmd[1] =0xff;
            transc->xfer_buf = cdc->data_cmd;
            transc->remain_len = 2U;
        }*/
		// 处理 CLASS 类型的请求
//		else if ((req->bmRequestType & USB_REQTYPE_MASK) == USB_REQTYPE_CLASS)
//		{
//			// 检查是哪个 CLASS 请求
//			switch (req->bRequest)
//			{
//				case SET_CONTROL_LINE_STATE:
//					// 这就是 DTR 信号！
//					// req->wValue 的 bit 0 (DTR)
//					if (req->wValue & 0x01) {
//						// DTR = 1, 上位机打开了COM口
//						cdc->dtr_enable = 1;
//					} else {
//						// DTR = 0, 上位机关闭了COM口
//						cdc->dtr_enable = 0;
//					}
//					break;

//				default:
//					break;
//			}
//		}
    }
		
    return USBD_OK;
}

static uint8_t cdc_ctlx_out (usb_dev *udev)
{
    if (udev->dev.class_core->alter_set != NO_CMD) {
        
        udev->dev.class_core->alter_set = NO_CMD;
    }

    return USBD_OK;
}


/*!
    \brief      handle CDC ACM data
    \param[in]  udev: pointer to USB device instance
    \param[in]  ep_num: endpoint identifier
    \param[out] none
    \retval     USB device operation status
*/
static uint8_t cdc_acm_in (usb_dev *udev, uint8_t ep_num)
{
    usb_cdc_handler *cdc = (usb_cdc_handler *)udev->dev.class_data[CDC_COM_INTERFACE];

    cdc->packet_sent = 1U;

    return USBD_OK;
}

/*!
    \brief      handle CDC ACM data
    \param[in]  udev: pointer to USB device instance
    \param[in]  ep_num: endpoint identifier
    \param[out] none
    \retval     USB device operation status
*/
static uint8_t cdc_acm_out (usb_dev *udev, uint8_t ep_num)
{
    usb_cdc_handler *cdc = (usb_cdc_handler *)udev->dev.class_data[CDC_COM_INTERFACE];

    cdc->packet_receive = 1U;
    cdc->receive_length = ((usb_core_driver *)udev)->dev.transc_out[ep_num].xfer_count;

    return USBD_OK;
}
