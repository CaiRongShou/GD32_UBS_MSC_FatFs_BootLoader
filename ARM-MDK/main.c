#include "main.h"  
#include "spi_flash.h"
#include "bsp_uart.h"
#include "bsp_sd.h"
#include "boot.h"
#include "bsp_usb.h"
#include "ff.h"


uint8_t flash_fatfs_test(void);
int main(void)
{
	board_init();
	
	/* 串口初始化 波特率：115200 */
	bsp_uart_init();
	
	/* SPI初始化 */
	SPI_FLASH_Init();
	
	//获取GD25Q32的设备ID
	printf("ID = %X\r\n",SPI_FLASH_ReadID());

    msc_ram_init();
    
    boot_main();
    while(1) 
	{

	}
}

uint8_t flash_fatfs_test(void)
{
	uint8_t i = 0;
    uint16_t BufferSize = 0;
    FIL	MyFile;			// 文件对象
    UINT 	MyFile_Num;		//	数据长度
    BYTE 	MyFile_WriteBuffer[] = "STM32H7B0 SD卡 文件系统测试";	//要写入的数据
    BYTE 	MyFile_ReadBuffer[4096];	//要读出的数据
    uint8_t MyFile_Res;    /* Return value for SD */
	MKFS_PARM mkfs_opt;
	mkfs_opt.align = 1;
	mkfs_opt.au_size = 4096;
	mkfs_opt.fmt = FM_FAT;
	mkfs_opt.n_fat = 1;
	mkfs_opt.n_root = 1;
    printf("-------------FatFs 文件创建和写入测试---------------\r\n");

    /* 挂载文件系统 */
    FATFS fs;
    MyFile_Res = f_mount(&fs, "1:", 1);
	if(MyFile_Res == FR_NO_FILESYSTEM) {
		/*creates an FAT volume on SPI FLASH(format)*/
		MyFile_Res = f_mkfs("1:", &mkfs_opt, (void *)MyFile_ReadBuffer,1024);
		/*unmount file system*/
		MyFile_Res = f_mount(NULL, "1:", 1);
		/*mount file system*/
		MyFile_Res = f_mount(&fs, "1:", 1);
	}
    if (MyFile_Res != FR_OK) {
        return MyFile_Res;
    }

    MyFile_Res = f_open(&MyFile,"1:Test.txt",FA_CREATE_ALWAYS | FA_WRITE);	//打开文件，若不存在则创建该文件
    if(MyFile_Res == FR_OK)
    {
        printf("文件打开/创建成功，准备写入数据...\r\n");

        MyFile_Res = f_write(&MyFile,MyFile_WriteBuffer,sizeof(MyFile_WriteBuffer),&MyFile_Num);	//向文件写入数据
        if (MyFile_Res == FR_OK)
        {
            printf("写入成功，写入内容为：\r\n");
            printf("%s\r\n",MyFile_WriteBuffer);
        }
        else
        {
            printf("文件写入失败，请检查SD卡或重新格式化!\r\n");
            f_close(&MyFile);	  //关闭文件
            return ERROR;
        }
        f_close(&MyFile);	  //关闭文件
    }
    else
    {
        printf("无法打开/创建文件，请检查SD卡或重新格式化!\r\n");
        f_close(&MyFile);	  //关闭文件
        return ERROR;
    }

    printf("-------------FatFs 文件读取测试---------------\r\n");

    BufferSize = sizeof(MyFile_WriteBuffer)/sizeof(BYTE);									// 计算写入的数据长度
    MyFile_Res = f_open(&MyFile,"1:Test.txt",FA_OPEN_EXISTING | FA_READ);	//打开文件，若不存在则创建该文件
    MyFile_Res = f_read(&MyFile,MyFile_ReadBuffer,BufferSize,&MyFile_Num);			// 读取文件
    if(MyFile_Res == FR_OK)
    {
        printf("文件读取成功，正在校验数据...\r\n");

        for(i=0; i<BufferSize; i++)
        {
            if(MyFile_WriteBuffer[i] != MyFile_ReadBuffer[i])		// 校验数据
            {
                printf("校验失败，请检查SD卡或重新格式化!\r\n");
                f_close(&MyFile);	  //关闭文件
                return ERROR;
            }
        }
        printf("校验成功，读出的数据为：\r\n");
        printf("%s\r\n",MyFile_ReadBuffer);
    }
    else
    {
        printf("无法读取文件，请检查SD卡或重新格式化!\r\n");
        f_close(&MyFile);	  //关闭文件
        return ERROR;
    }

    f_close(&MyFile);	  //关闭文件
    f_mount(NULL, "", 0);
    return SUCCESS;
}