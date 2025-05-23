#include "usbd_core.h"
#include "usbd_msc.h"
#include "bsp_sd.h"
#include "spi_flash.h"

#define MSC_IN_EP  0x81
#define MSC_OUT_EP 0x02

#define USBD_VID           0x5555
#define USBD_PID           0x6666
#define USBD_MAX_POWER     100
#define USBD_LANGID_STRING 1033

#define USB_CONFIG_SIZE (9 + MSC_DESCRIPTOR_LEN)

#ifdef CONFIG_USB_HS
#define MSC_MAX_MPS 512
#else
#define MSC_MAX_MPS 64
#endif

const uint8_t msc_ram_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0x00, 0x00, 0x00, USBD_VID, USBD_PID, 0x0200, 0x01),
    USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, 0x01, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    MSC_DESCRIPTOR_INIT(0x00, MSC_OUT_EP, MSC_IN_EP, MSC_MAX_MPS, 0x02),
    ///////////////////////////////////////
    /// string0 descriptor
    ///////////////////////////////////////
    USB_LANGID_INIT(USBD_LANGID_STRING),
    ///////////////////////////////////////
    /// string1 descriptor
    ///////////////////////////////////////
    0x14,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'C', 0x00,                  /* wcChar0 */
    'h', 0x00,                  /* wcChar1 */
    'e', 0x00,                  /* wcChar2 */
    'r', 0x00,                  /* wcChar3 */
    'r', 0x00,                  /* wcChar4 */
    'y', 0x00,                  /* wcChar5 */
    'U', 0x00,                  /* wcChar6 */
    'S', 0x00,                  /* wcChar7 */
    'B', 0x00,                  /* wcChar8 */
    ///////////////////////////////////////
    /// string2 descriptor
    ///////////////////////////////////////
    0x26,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'C', 0x00,                  /* wcChar0 */
    'h', 0x00,                  /* wcChar1 */
    'e', 0x00,                  /* wcChar2 */
    'r', 0x00,                  /* wcChar3 */
    'r', 0x00,                  /* wcChar4 */
    'y', 0x00,                  /* wcChar5 */
    'U', 0x00,                  /* wcChar6 */
    'S', 0x00,                  /* wcChar7 */
    'B', 0x00,                  /* wcChar8 */
    ' ', 0x00,                  /* wcChar9 */
    'M', 0x00,                  /* wcChar10 */
    'S', 0x00,                  /* wcChar11 */
    'C', 0x00,                  /* wcChar12 */
    ' ', 0x00,                  /* wcChar13 */
    'D', 0x00,                  /* wcChar14 */
    'E', 0x00,                  /* wcChar15 */
    'M', 0x00,                  /* wcChar16 */
    'O', 0x00,                  /* wcChar17 */
    ///////////////////////////////////////
    /// string3 descriptor
    ///////////////////////////////////////
    0x16,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    '2', 0x00,                  /* wcChar0 */
    '0', 0x00,                  /* wcChar1 */
    '2', 0x00,                  /* wcChar2 */
    '2', 0x00,                  /* wcChar3 */
    '1', 0x00,                  /* wcChar4 */
    '2', 0x00,                  /* wcChar5 */
    '3', 0x00,                  /* wcChar6 */
    '4', 0x00,                  /* wcChar7 */
    '5', 0x00,                  /* wcChar8 */
    '6', 0x00,                  /* wcChar9 */
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
    0x00
};

void usbd_event_handler(uint8_t event)
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
            break;
        case USBD_EVENT_SET_REMOTE_WAKEUP:
            break;
        case USBD_EVENT_CLR_REMOTE_WAKEUP:
            break;

        default:
            break;
    }
}

#define FLASH_BLOCK_SIZE  4096
#define SD_BLOCK_SIZE  512
#define FLASH_MSC   1
#define SD_MSC   0


void usbd_msc_get_cap(uint8_t lun, uint32_t *block_num, uint16_t *block_size)
{

#if SD_MSC
    *block_num = ((sd_cardinfo.card_csd.c_size + 1) * 1024);
    *block_size = sd_cardinfo.card_blocksize;
#endif

#if FLASH_MSC
    *block_num = 128*8;
    *block_size = 4096;
#endif
}
int usbd_msc_sector_read(uint32_t sector, uint8_t *buffer, uint32_t length)
{
#if SD_MSC
      uint8_t count = length / SD_BLOCK_SIZE;
    if (sector < ((sd_cardinfo.card_csd.c_size + 1) * 1024))
    {
        sd_block_read ((uint32_t *)buffer,sector * sd_cardinfo.card_blocksize,length);
        // if(count > 1) 
        //     sd_multiblocks_read((uint32_t *)buffer, sector * sd_cardinfo.card_blocksize, sd_cardinfo.card_blocksize, count);
        // else 
            // sd_block_read((uint32_t *)buffer, sector * sd_cardinfo.card_blocksize, sd_cardinfo.card_blocksize);
    }
#endif

#if FLASH_MSC
    if (sector < 128*8)    
    {
        SPI_FLASH_BufferRead(buffer,sector*FLASH_BLOCK_SIZE,length);
    }
#endif
    return 0;
}

int usbd_msc_sector_write(uint32_t sector, uint8_t *buffer, uint32_t length)
{
#if SD_MSC
    // if (sector < ((sd_cardinfo.card_csd.c_size + 1) * 1024)) //((sd_cardinfo.card_csd.c_size + 1) * 1024)
    {
       sd_block_write((uint32_t *)buffer, sector * sd_cardinfo.card_blocksize,length);
    //    uint8_t count = length / SD_BLOCK_SIZE;
    //    if(count > 1) 
    //         sd_multiblocks_write((uint32_t *)buffer, sector * sd_cardinfo.card_blocksize, sd_cardinfo.card_blocksize, count);
    //      else 
            // sd_block_write((uint32_t *)buffer, sector * sd_cardinfo.card_blocksize, sd_cardinfo.card_blocksize);
    }
#endif

#if FLASH_MSC
    if (sector < 128*8)    
    {
        SPI_FLASH_SectorErase(sector*FLASH_BLOCK_SIZE);	 
        SPI_FLASH_BufferWrite(buffer, sector*FLASH_BLOCK_SIZE ,length);
    }
#endif         
    return 0;
}

struct usbd_interface intf0;

void msc_ram_init(void)
{
    usbd_desc_register(msc_ram_descriptor);
    usbd_add_interface(usbd_msc_init_intf(&intf0, MSC_OUT_EP, MSC_IN_EP));

    usbd_initialize();
}
