/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"     /* Obtains integer types */
#include "diskio.h" /* Declarations of disk functions */
#include "bsp_spi_flash.h"
#include "bsp_sdio_sd.h"
#include "string.h"

/* Ϊÿ���豸����һ�������� */
#define ATA 0       // Ԥ��SD��ʹ��
#define SPI_FLASH 1 // �ⲿSPI Flash

#define SD_BLOCKSIZE 512

extern SD_CardInfo SDCardInfo;

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/
DSTATUS disk_status(BYTE pdrv)
{
    DSTATUS status = STA_NOINIT;

    switch (pdrv) {
    case ATA: /* SD CARD */
        status &= ~STA_NOINIT;
        break;

    case SPI_FLASH:
        /* SPI Flash״̬��⣺��ȡSPI Flash �豸ID */
        if (sFLASH_ID == SPI_FLASH_ReadID()) {
            /* �豸ID��ȡ�����ȷ */
            status &= ~STA_NOINIT;
        } else {
            /* �豸ID��ȡ������� */
            status = STA_NOINIT;
        }
        break;

    default:
        status = STA_NOINIT;
    }
    return status;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
    uint16_t i;
    DSTATUS status = STA_NOINIT;
    switch (pdrv) {
    case ATA: /* SD CARD */
        if (SD_Init() == SD_OK) {
            status &= ~STA_NOINIT;
        } else {
            status = STA_NOINIT;
        }

        break;

    case SPI_FLASH: /* SPI Flash */
        /* ��ʼ��SPI Flash */
        SPI_FLASH_Init();
        /* ��ʱһС��ʱ�� */
        i = 500;
        while (--i)
            ;
        /* ����SPI Flash */
        SPI_Flash_WAKEUP();
        /* ��ȡSPI FlashоƬ״̬ */
        status = disk_status(SPI_FLASH);
        break;

    default:
        status = STA_NOINIT;
    }
    return status;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(BYTE pdrv, /* Physical drive nmuber to identify the drive */
    BYTE *buff,              /* Data buffer to store read data */
    LBA_t sector,            /* Start sector in LBA */
    UINT count               /* Number of sectors to read */
)
{
    DRESULT status = RES_PARERR;
    SD_Error SD_state = SD_OK;

    switch (pdrv) {
    case ATA: /* SD CARD */
        if ((DWORD)buff & 3) {
            DRESULT res = RES_OK;
            DWORD scratch[SD_BLOCKSIZE / 4];

            while (count--) {
                res = disk_read(ATA, (void *)scratch, sector++, 1);

                if (res != RES_OK) {
                    break;
                }
                memcpy(buff, scratch, SD_BLOCKSIZE);
                buff += SD_BLOCKSIZE;
            }
            return res;
        }

        SD_state = SD_ReadMultiBlocks(buff, sector * SD_BLOCKSIZE, SD_BLOCKSIZE, count);
        if (SD_state == SD_OK) {
            /* Check if the Transfer is finished */
            SD_state = SD_WaitReadOperation();
            while (SD_GetStatus() != SD_TRANSFER_OK)
                ;
        }
        if (SD_state != SD_OK)
            status = RES_PARERR;
        else
            status = RES_OK;
        break;

    case SPI_FLASH:
        /* ����ƫ��6MB���ⲿFlash�ļ�ϵͳ�ռ����SPI Flash����10MB�ռ� */
        SPI_FLASH_BufferRead(buff, sector * 512, count * 512);
        status = RES_OK;
        break;

    default:
        status = RES_PARERR;
    }
    return status;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write(BYTE pdrv, /* Physical drive nmuber to identify the drive */
    const BYTE *buff,         /* Data to be written */
    LBA_t sector,             /* Start sector in LBA */
    UINT count                /* Number of sectors to write */
)
{
    uint32_t write_addr;
    DRESULT status = RES_PARERR;
    SD_Error SD_state = SD_OK;

    if (!count) {
        return RES_PARERR; /* Check parameter */
    }

    switch (pdrv) {
    case ATA: /* SD CARD */
        if ((DWORD)buff & 3) {
            DRESULT res = RES_OK;
            DWORD scratch[SD_BLOCKSIZE / 4];

            while (count--) {
                memcpy(scratch, buff, SD_BLOCKSIZE);
                res = disk_write(ATA, (void *)scratch, sector++, 1);
                if (res != RES_OK) {
                    break;
                }
                buff += SD_BLOCKSIZE;
            }
            return res;
        }

        SD_state = SD_WriteMultiBlocks((uint8_t *)buff, sector * SD_BLOCKSIZE, SD_BLOCKSIZE, count);
        if (SD_state == SD_OK) {
            /* Check if the Transfer is finished */
            SD_state = SD_WaitWriteOperation();

            /* Wait until end of DMA transfer */
            while (SD_GetStatus() != SD_TRANSFER_OK)
                ;
        }
        if (SD_state != SD_OK)
            status = RES_PARERR;
        else
            status = RES_OK;
        break;

    case SPI_FLASH:
        write_addr = sector * 512;
        SPI_FLASH_SectorErase(write_addr);
        SPI_FLASH_BufferWrite((u8 *)buff, write_addr, count * 512);
        status = RES_OK;
        break;

    default:
        status = RES_PARERR;
    }
    return status;
}

#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(BYTE pdrv, /* ������ */
    BYTE cmd,                 /* ����ָ�� */
    void *buff                /* д����߶�ȡ���ݵ�ַָ�� */
)
{
    DRESULT status = RES_PARERR;
    switch (pdrv) {
    case ATA: /* SD CARD */
        switch (cmd) {
        // Get R/W sector size (WORD)
        case GET_SECTOR_SIZE:
            *(WORD *)buff = SDCardInfo.CardBlockSize;
            break;
        // Get erase block size in unit of sector (DWORD)
        case GET_BLOCK_SIZE:
            *(DWORD *)buff = SDCardInfo.CardBlockSize; //SDCardInfo.CardBlockSize;
            break;

        case GET_SECTOR_COUNT:
            *(DWORD *)buff = SDCardInfo.CardCapacity / SDCardInfo.CardBlockSize;
            break;
        case CTRL_SYNC:
            break;
        }
        status = RES_OK;
        break;

    case SPI_FLASH:
        switch (cmd) {
        // Get R/W sector size (WORD)
        case GET_SECTOR_SIZE:
            *(WORD *)buff = 512;
            break;
        // Get erase block size in unit of sector (DWORD)
        case GET_BLOCK_SIZE:
            *(DWORD *)buff = 512; //SDCardInfo.CardBlockSize;
            break;

        case GET_SECTOR_COUNT:
            *(DWORD *)buff = (2 * 1024 * 1024) / 512;
            break;
        case CTRL_SYNC:
            break;
        }
        status = RES_OK;
        break;

    default:
        status = RES_PARERR;
        break;
    }
    return status;
}

DWORD get_fattime(void)
{
    /* ���ص�ǰʱ��� */
    return ((DWORD)(2015 - 1980) << 25) /* Year 2015 */
           | ((DWORD)1 << 21)           /* Month 1 */
           | ((DWORD)1 << 16)           /* Mday 1 */
           | ((DWORD)0 << 11)           /* Hour 0 */
           | ((DWORD)0 << 5)            /* Min 0 */
           | ((DWORD)0 >> 1);           /* Sec 0 */
}
