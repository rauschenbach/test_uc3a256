/*******************************************************************
Analog Devices, Inc. All Rights Reserved.
Description:    This file tests the SD interface on the EZ-KIT.
*******************************************************************/
#include <stdio.h>
#include <time.h>
#include <sd_mmc_spi.h>
#include "card.h"
#include "fatfs.h"
#include "asf.h"

/* Private functions ---------------------------------------------------------*/
/*---------------------------------------------------------*/
/* User Provided Timer Function for FatFs module           */
/*---------------------------------------------------------*/
uint32_t get_fattime(void)
{

    /*  Смотрим время  */
    return ((DWORD) (2014 - 1980) << 25)	/* Год  */
	|((DWORD) 3 << 21)	/* Месяц:           1...12  */
	|((DWORD) 17 << 16)	/* день:       1...30  */
	|((DWORD) 4 << 11)	/* часы:            0...23  */
	|((DWORD) 0 << 5)	/* минуты:          0...59  */
	|((DWORD) (14 / 2) >> 1);	/* секунды * 2: 0...29  */
}

/**
 * Опрашивать состояние диска, и если ее вынули-показать неготовность
 */
DSTATUS disk_status(BYTE drv)
{
#if 0
    if (!(*pPORTGIO & PG1))
	return STA_NOINIT;

    if (!(*pPORTGIO & PG0))
	return STA_PROTECT;
#endif
    return 0;
}


/**
 * Инициализация интерфейса SD карты. Порты, прерывания, DMA и проч.
 * Защиту от записи и наличие карты смотрим:
 * Не вставлена карта или какие-то проблемы,
 * идентификация типа карты
 */
DSTATUS disk_init(BYTE drv)
{
	uint32_t err, slot = 0;
    
	// Initialize SD MMC stack
    sd_mmc_init();


    printf("-- SD/MMC/SDIO Card Example --\n\r");
    printf("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);

    if (slot == sd_mmc_nb_slot()) {
	slot = 0;
    }
    printf("Please plug an SD, MMC or SDIO card in slot %d.\n\r", slot + 1);

    // Wait for a card and ready
    do {
	err = sd_mmc_check(slot);
	if ((SD_MMC_ERR_NO_CARD != err) && (SD_MMC_INIT_ONGOING != err) && (SD_MMC_OK != err)) {
	    printf("Card install FAILED\n\r");
	    printf("Please unplug and re-plug the card.\n\r");
	    while (SD_MMC_ERR_NO_CARD != sd_mmc_check(slot)) {
	    }
	}
    } while (SD_MMC_OK != err);
    return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
DRESULT disk_read(BYTE drv,	/* Physical drive nmuber (0) */
		  BYTE * buff,	/* Pointer to the data buffer to store read data */
		  DWORD sector,	/* Start sector number (LBA) */
		  BYTE count	/* Sector count (1..128) */
    )
{
    DSTATUS s;

    s = disk_status(drv);
    if (s & STA_NOINIT)
	return RES_NOTRDY;
    if (!count)
	return RES_PARERR;

    /* Прочитать блок / блоки  */
/////   rsi_read_blocks_from_sd_card(sector, buff, count);


    // Read the last block
    if (SD_MMC_OK != sd_mmc_init_read_blocks(0, sector, count)) {
	return RES_NOTRDY;
    }

    if (SD_MMC_OK != sd_mmc_start_read_blocks(buff, count)) {
	return RES_NOTRDY;
    }
    if (SD_MMC_OK != sd_mmc_wait_end_of_read_blocks()) {
	return RES_NOTRDY;
    }

    return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
DRESULT disk_write(BYTE drv,	/* Physical drive nmuber (0) */
		   const BYTE * buff,	/* Pointer to the data to be written */
		   DWORD sector,	/* Start sector number (LBA) */
		   BYTE count	/* Sector count (1..128) */
    )
{

    DSTATUS s;


    s = disk_status(drv);
    if (s & STA_NOINIT)
	return RES_NOTRDY;
    if (s & STA_PROTECT)
	return RES_WRPRT;	/* пока не проверяем это */
    if (!count)
	return RES_PARERR;

    /* Записать блок / блоки  */
//    rsi_write_blocks_to_sd_card(sector, (void *) buff, count);

    if (SD_MMC_OK != sd_mmc_init_write_blocks(0, sector, count)) {
	return RES_WRPRT;
    }

    if (SD_MMC_OK != sd_mmc_start_write_blocks(buff, count)) {
	return RES_WRPRT;
    }
    if (SD_MMC_OK != sd_mmc_wait_end_of_write_blocks()) {
	return RES_WRPRT;
    }


    return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/
DRESULT disk_ioctl(BYTE drv,	/* Physical drive nmuber (0) */
		   BYTE ctrl,	/* Control code */
		   void *buff	/* Buffer to send/receive control data */
    )
{
    DRESULT res;
    BYTE n, csd[16];
    WORD cs;

    if (disk_status(drv) & STA_NOINIT)	/* Check if card is in the socket */
	return RES_NOTRDY;

    res = RES_ERROR;
    switch (ctrl) {
    case CTRL_SYNC:		/* Make sure that no pending write process */
	res = RES_OK;
	break;

	/* Get number of sectors on the disk (DWORD) */
    case GET_SECTOR_COUNT:
	cs = sd_mmc_get_capacity(0) / 1024;	// в МБайтах
	//              rsi_get_sec_count();
	*(DWORD *) buff = (DWORD) cs / 512;
	res = RES_OK;
	break;

    case GET_BLOCK_SIZE:	/* Get erase block size in unit of sector (DWORD) */
	*(DWORD *) buff = 128;
	res = RES_OK;
	break;

    default:
	res = RES_PARERR;
    }

    return res;
}
