/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "bsp_spi_flash.h"
#include "string.h"

/* 为每个设备定义一个物理编号 */
#define ATA			    0     // 预留SD卡使用
#define SPI_FLASH		1     // 外部SPI Flash


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/
DSTATUS disk_status(BYTE pdrv) {
    DSTATUS status = STA_NOINIT;
	
	switch (pdrv) {
		case ATA:	/* SD CARD */
			break;
    
		case SPI_FLASH:      
      /* SPI Flash状态检测：读取SPI Flash 设备ID */
      if(sFLASH_ID == SPI_FLASH_ReadID())
      {
        /* 设备ID读取结果正确 */
        status &= ~STA_NOINIT;
      }
      else
      {
        /* 设备ID读取结果错误 */
        status = STA_NOINIT;;
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

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	uint16_t i;
	DSTATUS status = STA_NOINIT;	
	switch (pdrv) {
	case ATA:	         /* SD CARD */
		break;

	case SPI_FLASH:    /* SPI Flash */ 
		/* 初始化SPI Flash */
		SPI_FLASH_Init();
		/* 延时一小段时间 */
		i=500;
		while(--i);	
		/* 唤醒SPI Flash */
		SPI_Flash_WAKEUP();
		/* 获取SPI Flash芯片状态 */
		status=disk_status(SPI_FLASH);
		break;

	default:
		status = STA_NOINIT;
	}
	return status;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
   	DRESULT status = RES_PARERR;
	switch (pdrv) {
		case ATA:	/* SD CARD */
		break;

	case SPI_FLASH:
  /* 扇区偏移6MB，外部Flash文件系统空间放在SPI Flash后面10MB空间 */    
  SPI_FLASH_BufferRead(buff, sector <<12, count<<12);
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

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
   uint32_t write_addr; 
	DRESULT status = RES_PARERR;
	if (!count) {
		return RES_PARERR;		/* Check parameter */
	}

	switch (pdrv) {
		case ATA:	/* SD CARD */      
		break;

		case SPI_FLASH:
      /* 扇区偏移6MB，外部Flash文件系统空间放在SPI Flash后面10MB空间 */
			sector+=1536;
      write_addr = sector<<12;    
      SPI_FLASH_SectorErase(write_addr);
      SPI_FLASH_BufferWrite((u8 *)buff,write_addr,count<<12);
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

DRESULT disk_ioctl (
	BYTE pdrv,		/* 物理编号 */
	BYTE cmd,		  /* 控制指令 */
	void *buff		/* 写入或者读取数据地址指针 */
)
{
	DRESULT status = RES_PARERR;
	switch (pdrv) {
	case ATA:	/* SD CARD */
		break;

	case SPI_FLASH:
		switch (cmd) {
		/* 扇区数量：2560*4096/1024/1024=10(MB) */
		case GET_SECTOR_COUNT:
			*(DWORD * )buff = 512;		
			break;
		/* 扇区大小  */
		case GET_SECTOR_SIZE :
			*(WORD * )buff = 4096;
			break;
		/* 同时擦除扇区个数 */
		case GET_BLOCK_SIZE :
			*(DWORD * )buff = 1;
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

__weak DWORD get_fattime(void) {
	/* 返回当前时间戳 */
	return	  ((DWORD)(2015 - 1980) << 25)	/* Year 2015 */
			| ((DWORD)1 << 21)				/* Month 1 */
			| ((DWORD)1 << 16)				/* Mday 1 */
			| ((DWORD)0 << 11)				/* Hour 0 */
			| ((DWORD)0 << 5)				  /* Min 0 */
			| ((DWORD)0 >> 1);				/* Sec 0 */
}

