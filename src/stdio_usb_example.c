#include <asf.h>
#include <board.h>
#include <string.h>
#include <sysclk.h>
#include <stdio_usb.h>
#include "conf_board.h"
#include "conf_clock.h"
#include "fatfs.h"
#include "sd_mmc_protocol.h"
#include "time_tick.h"

void die(char* str, int rc)
{
    printf("%s failed with rc = %d\r\n", str, rc);
    LED_On(LED2);
    for (;;);
}

/**
  * @brief  Скинуть файл на карту
  * @param  None
  * @retval None
  */
static void DATA_Flush(void)
{
    char name[24];
    char buf[255];
    char *str = "TEST: erik ist klug!";
    int i = 0;
    FATFS fatfs;		/* File system object */
    FRESULT rc;			/* Result code */
    FIL fil;			/* File object */
    unsigned j, bw;

    if ((rc = f_mount(0, &fatfs)) == 0)
	printf("Работа с SD картой в режиме SPI, mount FAT32 OK\r\n");
    else
	die("mount", rc);


    for (i = 0; i < 16; i++) {
	sprintf(name, "data_%03d", i);

   LED_Toggle(LED1);

	rc = f_open(&fil, name, FA_WRITE | FA_CREATE_ALWAYS);
	if (rc == 0)
	    printf("Create a new file %s OK\r\n", name);
	else
	    die("open", rc);


	printf("...Write a text data... ");
	rc = f_write(&fil, str, strlen(str), &bw);
	if (rc == 0)
	    printf(" %d bytes written...\r\n", bw);
	else
	    die("write", rc);


	printf("Close the file.\r\n");
	rc = f_close(&fil);
	if (rc)
	    die("close", rc);
    }

	rc = f_open(&fil, "data_000", FA_READ | FA_OPEN_EXISTING);
	if (rc == 0)
	    printf("Open file %s OK\r\n", name);
	else
	    die("open", rc);


	do {
	rc = f_read(&fil, buf, sizeof(buf), &bw);
	if (rc == 0)
	    printf("%s", buf);
	} while(rc);

	rc = f_close(&fil);
	if (rc)
	    die("close", rc);
}



int main(void)
{
    uint8_t slot = 0;
    sd_mmc_err_t err;
    int i, j = 0;

    sysclk_init();
    board_init();
    irq_initialize_vectors();
    cpu_irq_enable();
    stdio_usb_init();

    while (j++ < 20) {
	printf("test %d\r\n", j);
	LED_Toggle(LED1);
	for (i = 0; i < 0x80000; i++);
    }

	DATA_Flush();
	
	printf("\nTest completed.\n");
	LED_On(LED1);
    while(1);
}



