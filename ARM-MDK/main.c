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
	
	/* ���ڳ�ʼ�� �����ʣ�115200 */
	bsp_uart_init();
	
	/* SPI��ʼ�� */
	SPI_FLASH_Init();
	
	//��ȡGD25Q32���豸ID
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
    FIL	MyFile;			// �ļ�����
    UINT 	MyFile_Num;		//	���ݳ���
    BYTE 	MyFile_WriteBuffer[] = "STM32H7B0 SD�� �ļ�ϵͳ����";	//Ҫд�������
    BYTE 	MyFile_ReadBuffer[4096];	//Ҫ����������
    uint8_t MyFile_Res;    /* Return value for SD */
	MKFS_PARM mkfs_opt;
	mkfs_opt.align = 1;
	mkfs_opt.au_size = 4096;
	mkfs_opt.fmt = FM_FAT;
	mkfs_opt.n_fat = 1;
	mkfs_opt.n_root = 1;
    printf("-------------FatFs �ļ�������д�����---------------\r\n");

    /* �����ļ�ϵͳ */
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

    MyFile_Res = f_open(&MyFile,"1:Test.txt",FA_CREATE_ALWAYS | FA_WRITE);	//���ļ������������򴴽����ļ�
    if(MyFile_Res == FR_OK)
    {
        printf("�ļ���/�����ɹ���׼��д������...\r\n");

        MyFile_Res = f_write(&MyFile,MyFile_WriteBuffer,sizeof(MyFile_WriteBuffer),&MyFile_Num);	//���ļ�д������
        if (MyFile_Res == FR_OK)
        {
            printf("д��ɹ���д������Ϊ��\r\n");
            printf("%s\r\n",MyFile_WriteBuffer);
        }
        else
        {
            printf("�ļ�д��ʧ�ܣ�����SD�������¸�ʽ��!\r\n");
            f_close(&MyFile);	  //�ر��ļ�
            return ERROR;
        }
        f_close(&MyFile);	  //�ر��ļ�
    }
    else
    {
        printf("�޷���/�����ļ�������SD�������¸�ʽ��!\r\n");
        f_close(&MyFile);	  //�ر��ļ�
        return ERROR;
    }

    printf("-------------FatFs �ļ���ȡ����---------------\r\n");

    BufferSize = sizeof(MyFile_WriteBuffer)/sizeof(BYTE);									// ����д������ݳ���
    MyFile_Res = f_open(&MyFile,"1:Test.txt",FA_OPEN_EXISTING | FA_READ);	//���ļ������������򴴽����ļ�
    MyFile_Res = f_read(&MyFile,MyFile_ReadBuffer,BufferSize,&MyFile_Num);			// ��ȡ�ļ�
    if(MyFile_Res == FR_OK)
    {
        printf("�ļ���ȡ�ɹ�������У������...\r\n");

        for(i=0; i<BufferSize; i++)
        {
            if(MyFile_WriteBuffer[i] != MyFile_ReadBuffer[i])		// У������
            {
                printf("У��ʧ�ܣ�����SD�������¸�ʽ��!\r\n");
                f_close(&MyFile);	  //�ر��ļ�
                return ERROR;
            }
        }
        printf("У��ɹ�������������Ϊ��\r\n");
        printf("%s\r\n",MyFile_ReadBuffer);
    }
    else
    {
        printf("�޷���ȡ�ļ�������SD�������¸�ʽ��!\r\n");
        f_close(&MyFile);	  //�ر��ļ�
        return ERROR;
    }

    f_close(&MyFile);	  //�ر��ļ�
    f_mount(NULL, "", 0);
    return SUCCESS;
}