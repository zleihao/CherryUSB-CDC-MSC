#include "RTOS.h"
#include "usb_config.h"
#include "stm32f4xx.h"
#include "usbd_core.h"
#include "usbd_msc.h"
#include "usbd_cdc_acm.h"
#include "chry_ringbuffer.h"
#include "ff.h"
#include "diskio.h"
#include "usart.h"

#define MSC_IN_EP 0x81
#define MSC_OUT_EP 0x02

#define CDC_INT_EP 0x82
#define CDC_IN_EP 0x83
#define CDC_OUT_EP 0x03

#define USBD_VID 0xFFFF
#define USBD_PID 0xFFFF
#define USBD_MAX_POWER 100
#define USBD_LANGID_STRING 1033

#define INT_NUM 3

#define USB_CONFIG_SIZE (9 + MSC_DESCRIPTOR_LEN + CDC_ACM_DESCRIPTOR_LEN)

#ifdef CONFIG_USB_HS
#define MSC_MAX_MPS 512
#else
#define MSC_MAX_MPS 64
#define CDC_MAX_MPS 64
#endif

const uint8_t cdc_msc_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0x00, 0x00, 0x00, USBD_VID, USBD_PID, 0x0200, 0x01),
    USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, INT_NUM, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    MSC_DESCRIPTOR_INIT(0x00, MSC_OUT_EP, MSC_IN_EP, MSC_MAX_MPS, 0x02),
    CDC_ACM_DESCRIPTOR_INIT(0x01, CDC_INT_EP, CDC_OUT_EP, CDC_IN_EP, CDC_MAX_MPS, 0x02),

    ///////////////////////////////////////
    /// string0 descriptor
    ///////////////////////////////////////
    USB_LANGID_INIT(USBD_LANGID_STRING),
    ///////////////////////////////////////
    /// string1 descriptor
    ///////////////////////////////////////
    0x14,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'C',
    0x00, /* wcChar0 */
    'h',
    0x00, /* wcChar1 */
    'e',
    0x00, /* wcChar2 */
    'r',
    0x00, /* wcChar3 */
    'r',
    0x00, /* wcChar4 */
    'y',
    0x00, /* wcChar5 */
    'U',
    0x00, /* wcChar6 */
    'S',
    0x00, /* wcChar7 */
    'B',
    0x00, /* wcChar8 */
    ///////////////////////////////////////
    /// string2 descriptor
    ///////////////////////////////////////
    0x26,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'C',
    0x00, /* wcChar0 */
    'h',
    0x00, /* wcChar1 */
    'e',
    0x00, /* wcChar2 */
    'r',
    0x00, /* wcChar3 */
    'r',
    0x00, /* wcChar4 */
    'y',
    0x00, /* wcChar5 */
    'U',
    0x00, /* wcChar6 */
    'S',
    0x00, /* wcChar7 */
    'B',
    0x00, /* wcChar8 */
    ' ',
    0x00, /* wcChar9 */
    'M',
    0x00, /* wcChar10 */
    'S',
    0x00, /* wcChar11 */
    'C',
    0x00, /* wcChar12 */
    ' ',
    0x00, /* wcChar13 */
    'D',
    0x00, /* wcChar14 */
    'E',
    0x00, /* wcChar15 */
    'M',
    0x00, /* wcChar16 */
    'O',
    0x00, /* wcChar17 */
    ///////////////////////////////////////
    /// string3 descriptor
    ///////////////////////////////////////
    0x16,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    '2',
    0x00, /* wcChar0 */
    '0',
    0x00, /* wcChar1 */
    '2',
    0x00, /* wcChar2 */
    '2',
    0x00, /* wcChar3 */
    '1',
    0x00, /* wcChar4 */
    '2',
    0x00, /* wcChar5 */
    '3',
    0x00, /* wcChar6 */
    '4',
    0x00, /* wcChar7 */
    '5',
    0x00, /* wcChar8 */
    '6',
    0x00, /* wcChar9 */
#ifdef CONFIG_USB_HS
    ///////////////////////////////////////
    /// device qualifier descriptor
    ///////////////////////////////////////
    0x0a,
    USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER,
    0x00,
    0x02,
    0x00,
    0x00,
    0x00,
    0x40,
    0x01,
    0x00,
#endif
    0x00};

static struct cdc_line_coding *g_cdc_coding;

/* usb cdc 相关参数 */
uint8_t g_usb2uart_config;
uint8_t g_usb2uart_transfer;

uint8_t g_usb_rx_idle_flag;
uint8_t g_usb_tx_idle_flag;
uint8_t g_uart_rx_idle_flag;
uint8_t g_uart_tx_idle_flag;

#define CONFIG_UARTRX_RINGBUF_SIZE (4 * 1024)
#define CONFIG_USBRX_RINGBUF_SIZE (1 * 1024)

chry_ringbuffer_t g_usb_rx;
chry_ringbuffer_t g_uart_rx;
uint8_t usb_rx_ringbuffer[CONFIG_USBRX_RINGBUF_SIZE];
uint8_t uart_rx_ringbuffer[CONFIG_UARTRX_RINGBUF_SIZE];

__attribute__((aligned(4))) static uint8_t _usbtx_buffer[CONFIG_UARTRX_RINGBUF_SIZE];

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t
    usb_tmpbuffer[64]; /* 2048 is only for test speed , please use CDC_MAX_MPS for common*/
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t write_buffer[2048];

volatile bool ep_tx_busy_flag = false;

static void usbd_event_handler(uint8_t busid, uint8_t event)
{
    switch (event) {
    case USBD_EVENT_RESET:
        break;
    case USBD_EVENT_CONNECTED:
        break;
    case USBD_EVENT_DISCONNECTED:
        break;
    case USBD_EVENT_RESUME:
        break;
    case USBD_EVENT_SUSPEND:
        break;
    case USBD_EVENT_CONFIGURED:
        ep_tx_busy_flag = false;
        /* setup first out ep read transfer */
        usbd_ep_start_read(busid, CDC_OUT_EP, usb_tmpbuffer, 64);
        break;
    case USBD_EVENT_SET_REMOTE_WAKEUP:
        break;
    case USBD_EVENT_CLR_REMOTE_WAKEUP:
        break;

    default:
        break;
    }
}

/* MSC */

void usbd_msc_get_cap(uint8_t busid, uint8_t lun, uint32_t *block_num, uint32_t *block_size)
{
    *block_size = 512;                              // USB MSC 规范，扇区大小固定 512B
    *block_num = (2 * 1024 * 1024) / (*block_size); // 计算扇区总数 (2MB / 512)
}
int usbd_msc_sector_read(uint8_t busid, uint8_t lun, uint32_t sector, uint8_t *buffer, uint32_t length)
{
    return disk_read(1, buffer, sector, length / 512) == FR_OK ? 0 : 1;
}

int usbd_msc_sector_write(uint8_t busid, uint8_t lun, uint32_t sector, uint8_t *buffer, uint32_t length)
{
    return disk_write(1, buffer, sector, length / 512) == FR_OK ? 0 : 1;
}

static struct usbd_interface intf0;

/* CDC */
void usbd_cdc_acm_bulk_out(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    /* 当执行到该函数时，本质上数据已经在usb_tmpbuffer了 */
    //写到g_usb_rx中
    chry_ringbuffer_write(&g_usb_rx, usb_tmpbuffer, nbytes);

    if (chry_ringbuffer_get_free(&g_usb_rx) > 64) {
        usbd_ep_start_read(busid, CDC_OUT_EP, usb_tmpbuffer, 64);
    }
    else {
        g_usb_rx_idle_flag = 1;
    }
}

void usbd_cdc_acm_bulk_in(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    uint32_t size;
    uint8_t *buffer;

    chry_ringbuffer_linear_read_done(&g_uart_rx, nbytes);
    if ((nbytes % usbd_get_ep_mps(busid, ep)) == 0 && nbytes) {
        /* send zlp */
        usbd_ep_start_write(busid, CDC_IN_EP, NULL, 0);
    }
    else {
        if (chry_ringbuffer_get_used(&g_uart_rx)) {
            buffer = chry_ringbuffer_linear_read_setup(&g_uart_rx, &size);

            memcpy(_usbtx_buffer, buffer, size);

            usbd_ep_start_write(busid, CDC_IN_EP, _usbtx_buffer, size);
        }
        else {
            g_usb_tx_idle_flag = 1;
        }
    }
}

/*!< endpoint call back */
struct usbd_endpoint cdc_out_ep = {.ep_addr = CDC_OUT_EP, .ep_cb = usbd_cdc_acm_bulk_out};

struct usbd_endpoint cdc_in_ep = {.ep_addr = CDC_IN_EP, .ep_cb = usbd_cdc_acm_bulk_in};

static struct usbd_interface intf1;
static struct usbd_interface intf2;

void cdc_msc_init(uint8_t busid, uintptr_t reg_base)
{
    chry_ringbuffer_init(&g_uart_rx, uart_rx_ringbuffer, CONFIG_UARTRX_RINGBUF_SIZE);
    chry_ringbuffer_init(&g_usb_rx, usb_rx_ringbuffer, CONFIG_USBRX_RINGBUF_SIZE);

    usbd_desc_register(busid, cdc_msc_descriptor);
    usbd_add_interface(busid, usbd_msc_init_intf(busid, &intf0, MSC_OUT_EP, MSC_IN_EP));

    usbd_add_interface(busid, usbd_cdc_acm_init_intf(busid, &intf1));
    usbd_add_interface(busid, usbd_cdc_acm_init_intf(busid, &intf2));
    usbd_add_endpoint(busid, &cdc_out_ep);
    usbd_add_endpoint(busid, &cdc_in_ep);

    usbd_initialize(busid, reg_base, usbd_event_handler);
}

#if defined(CONFIG_USBDEV_MSC_POLLING)
void msc_ram_polling(uint8_t busid)
{
    usbd_msc_polling(busid);
}
#endif

/* CDC Hanlder Function */
void usbd_cdc_acm_set_line_coding(uint8_t busid, uint8_t intf, struct cdc_line_coding *line_coding)
{
    if (memcmp(line_coding, (uint8_t *)&g_cdc_coding, sizeof(struct cdc_line_coding)) != 0) {
        memcpy((uint8_t *)&g_cdc_coding, line_coding, sizeof(struct cdc_line_coding));

        /* 启动串口配置 */
        g_usb2uart_config = 1;
        g_usb2uart_transfer = 0;
    }
}

void usbd_cdc_acm_get_line_coding(uint8_t busid, uint8_t intf, struct cdc_line_coding *line_coding)
{
    memcpy(line_coding, (uint8_t *)&g_cdc_coding, sizeof(struct cdc_line_coding));
}

extern void usb2uart_uart_config_callback(struct cdc_line_coding *line_coding);

void usb2uart_handler(uint8_t busid)
{
    uint32_t size;
    uint8_t *buffer;

    if (g_usb2uart_config) {
        g_usb2uart_config = 0;
        /* 开始配置 */
        usb2uart_uart_config_callback((struct cdc_line_coding *)&g_cdc_coding);
        g_usb_tx_idle_flag = 1;
        g_uart_tx_idle_flag = 1;
        g_usb2uart_transfer = 1;
    }

    if (0 == g_usb2uart_transfer) {
        return;
    }

    /* usb tx idle */
    if (g_usb_tx_idle_flag) {
        if (chry_ringbuffer_get_used(&g_uart_rx)) { //判断uart rx是否接收到数据
            g_usb_tx_idle_flag = 0;
            //意味着可以把uart接收到的数据写到usb in endpoint
            buffer = chry_ringbuffer_linear_read_setup(&g_uart_rx, &size);

            memcpy(_usbtx_buffer, buffer, size);

            usbd_ep_start_write(busid, CDC_IN_EP, _usbtx_buffer, size);
        }
    }

    /* uart tx idle */
    if (g_uart_tx_idle_flag) {
        if (chry_ringbuffer_get_used(&g_usb_rx)) { //判断uart rx是否接收到数据
            g_uart_rx_idle_flag = 0;
            //意味着可以把uart接收到的数据写到usb in endpoint
            buffer = chry_ringbuffer_linear_read_setup(&g_usb_rx, &size);

            /* 通过uart tx发送出去 */
            usb2uart_uart_send_bydma(buffer, size);
        }
    }

    if (g_usb_rx_idle_flag) {
        if (chry_ringbuffer_get_free(&g_usb_rx) >= 64) {
            g_usb_rx_idle_flag = 0;
            usbd_ep_start_read(busid, CDC_OUT_EP, usb_tmpbuffer, 64);
        }
    }
}

/* called by user */
void chry_dap_usb2uart_uart_send_complete(uint32_t size)
{
    uint8_t *buffer;

    chry_ringbuffer_linear_read_done(&g_usb_rx, size);

    if (chry_ringbuffer_get_used(&g_usb_rx)) {
        //意味着可以把uart接收到的数据写到usb in endpoint
        buffer = chry_ringbuffer_linear_read_setup(&g_usb_rx, &size);

        /* 通过uart tx发送出去 */
        usb2uart_uart_send_bydma(buffer, size);
    }
    else {
        g_uart_tx_idle_flag = 1;
    }
}

/* 创建 usb 任务 */
static StaticTask_t usbd_task_tcb;

static void usbd_task_handler(void *param)
{
    cdc_msc_init(0, 0x50000000UL);

    while (1) {
        usb2uart_handler(0);
        vTaskDelay(3);
    }
}

void usbd_task_handler_init()
{
    xTaskCreate(usbd_task_handler, "usbd_task_handler", 128, NULL, 4, (TaskHandle_t *)&usbd_task_tcb);
}
INIT_APP_TASK(usbd_task_handler_init);

//====================================================================
