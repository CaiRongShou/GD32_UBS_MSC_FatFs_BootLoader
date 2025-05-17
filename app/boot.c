#include "boot.h"
#include "bsp_fmc.h"
#include "ff.h"
#include <stdio.h>


#define APP_ADDR  0x08010000    /* APP��ַ */


FATFS fs;
FILINFO fno;
FIL file;
char DiskPath[4] = {"1:\\"};                      /* SD���߼�����·�������̷�0������"0:/" */
// MKFS_PARM mkfs_opt = 
// {
// 	.align = 1,
// 	.au_size = 4096,
// 	.fmt = FM_FAT,
// };

__IO uint32_t uwCRCValue;
__IO uint32_t uwExpectedCRCValue;
__IO uint32_t uwAppSize;

uint8_t tempbuf[4096];  /* ��ȡ���ݵĻ��� */

typedef void (*app_jump)(void);

static void disp_menu(void);
static void bsp_deinit(void);
static void jump_to_app(void);
static void boot_load_firmware(void);
static void boot_hex_crc(void);
static void view_root_dir(void);


/* FatFs API的返回值 */
static const char * FR_Table[]= 
{
	"FR_OK：成功",				                             /* (0) Succeeded */
	"FR_DISK_ERR：底层硬件错误",			                 /* (1) A hard error occurred in the low level disk I/O layer */
	"FR_INT_ERR：断言失败",				                     /* (2) Assertion failed */
	"FR_NOT_READY：物理驱动没有工作",			             /* (3) The physical drive cannot work */
	"FR_NO_FILE：文件不存在",				                 /* (4) Could not find the file */
	"FR_NO_PATH：路径不存在",				                 /* (5) Could not find the path */
	"FR_INVALID_NAME：无效文件名",		                     /* (6) The path name format is invalid */
	"FR_DENIED：由于禁止访问或者目录已满访问被拒绝",         /* (7) Access denied due to prohibited access or directory full */
	"FR_EXIST：文件已经存在",			                     /* (8) Access denied due to prohibited access */
	"FR_INVALID_OBJECT：文件或者目录对象无效",		         /* (9) The file/directory object is invalid */
	"FR_WRITE_PROTECTED：物理驱动被写保护",		             /* (10) The physical drive is write protected */
	"FR_INVALID_DRIVE：逻辑驱动号无效",		                 /* (11) The logical drive number is invalid */
	"FR_NOT_ENABLED：卷中无工作区",			                 /* (12) The volume has no work area */
	"FR_NO_FILESYSTEM：没有有效的FAT卷",		             /* (13) There is no valid FAT volume */
	"FR_MKFS_ABORTED：由于参数错误f_mkfs()被终止",	         /* (14) The f_mkfs() aborted due to any parameter error */
	"FR_TIMEOUT：在规定的时间内无法获得访问卷的许可",		 /* (15) Could not get a grant to access the volume within defined period */
	"FR_LOCKED：由于文件共享策略操作被拒绝",				 /* (16) The operation is rejected according to the file sharing policy */
	"FR_NOT_ENOUGH_CORE：无法分配长文件名工作区",		     /* (17) LFN working buffer could not be allocated */
	"FR_TOO_MANY_OPEN_FILES：当前打开的文件数大于_FS_SHARE", /* (18) Number of open files > _FS_SHARE */
	"FR_INVALID_PARAMETER：参数无效"	                     /* (19) Given parameter is invalid */
};

 void MSP_SP(uint32_t addr)
{
	__asm volatile("MSR msp ,r0");
	__asm volatile("BX r14");
}

void boot_main(void)
{
    disp_menu();
    while (1)
    {
        switch (getchar())
        {
        case '1':
		printf("【1 - 显示SD卡根目录下的文件名】\r\n");
            view_root_dir();
         break;
        case '2':
        printf("【2 - 启动固件加载, 校验和跳转】\r\n");
        boot_load_firmware();
        boot_hex_crc();
        jump_to_app();
        break;
        default:
            disp_menu();
        break;
        }
    }
    
}


/*
*********************************************************************************************************
*	函 数 名: DispMenu
*	功能说明: 显示操作提示菜单
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void disp_menu(void)
{
	printf("请选择操作命令:\r\n");
	printf("1 - 显示SD卡根目录下的文件名\r\n");
	printf("2 - 启动固件加载, 校验和跳转\r\n");
}

static void bsp_deinit(void)
{
    SysTick->LOAD  = 0;                        
    SysTick->VAL   = 0UL;                                            
    SysTick->CTRL  = 0;

    rcu_deinit();
    
    gpio_deinit(GPIOA);
    gpio_deinit(GPIOB);
    gpio_deinit(GPIOC);
    gpio_deinit(GPIOD);

    spi_i2s_deinit(SPI0);
    usart_deinit(USART0);

    nvic_irq_disable(USBFS_IRQn);
    nvic_irq_disable(SDIO_IRQn);
}
/*
*********************************************************************************************************
*	函 数 名: JumpToApp
*	功能说明: 跳转到应用JumpToApp
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/static void jump_to_app(void)
{
    app_jump app;
    __disable_irq();
    bsp_deinit();
    __enable_irq();

    /* 跳转到应用程序，首地址是MSP，地址+4是复位中断服务程序地址 */
    app = (app_jump)(*((uint32_t *)(APP_ADDR + 4)));
    /* 设置主堆栈指针 */
    MSP_SP(*((uint32_t *)APP_ADDR));
    // /* 在RTOS工程，这条语句很重要，设置为特权级模式，使用MSP指针 */
	// __set_CONTROL(0);
    app();
    /* 跳转成功的话，不会执行到这里，用户可以在这里添加代码 */
	while (1)
	{

	}
}
/*
*********************************************************************************************************
*	函 数 名: BootHexCrcVeriy
*	功能说明: 固件CRC校验
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void boot_hex_crc(void)
{
    /* 读取bin文件的CRC */
	uwExpectedCRCValue  = *(__IO uint32_t *)(APP_ADDR + uwAppSize - 4);

    crc_data_register_reset();
    uwCRCValue = crc_block_data_calculate((uint32_t *)(APP_ADDR), uwAppSize/4 - 1);
    printf("实际APP固件CRC校验值 = 0x%x\r\n", uwExpectedCRCValue);
	printf("计算APP固件CRC校验值 = 0x%x\r\n", uwCRCValue);	
	
	if (uwCRCValue != uwExpectedCRCValue)
	{
		printf("校验失败\r\n");
        //while(1);
	}
	else
	{
		printf("校验通过，开始跳转到APP\r\n");
	}
	
	printf("=================================================\r\n");

}
/*
*********************************************************************************************************
*	函 数 名: LoadFirmware
*	功能说明: 固件加载
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/static void boot_load_firmware(void)
{
    char file_path[64];
    uint8_t result;
    uint32_t bw;
    uint32_t ver;
    uint8_t SectorCount = 0;
	uint8_t SectorRemain = 0;
    uint8_t i;
    uint32_t Count = 0;
    uint32_t TotalSize = 0;
    float FinishPecent;

	result = f_mount(&fs, DiskPath , 1);	/* Mount a logical drive */
    if (result != FR_OK)
	{
		printf("挂载文件系统失败 (%s)\r\n", FR_Table[result]);
        return;
	}

	sprintf(file_path,"%sapp.bin",DiskPath);
    result = f_open(&file,file_path,FA_OPEN_EXISTING | FA_READ);
    if (result != FR_OK)
	{
		printf("Don't Find File : app.bin\r\n");
        return;
	}

	f_stat(file_path,&fno);
    uwAppSize = fno.fsize;
    printf("APP固件大小：%d\r\n", (int)fno.fsize);
    f_lseek(&file,28);
    f_read(&file,&ver,sizeof(ver),&bw);
    f_lseek(&file,0);
    printf("APP固件版本：V%X.%02X\r\n", ver >> 8, ver & 0xFF);

	SectorCount = uwAppSize / (64 * 1024);
    SectorRemain = uwAppSize % (64 * 1024);

    for(i=0;i<SectorCount;i++)
    {
        printf("开始擦除扇区 = %08x\r\n", APP_ADDR + i*64*1024);
		fmc_erase_sector(fmc_sector_get((uint32_t)(APP_ADDR + i*64*1024)));
    }
    if(SectorRemain)
    {
        printf("开始擦除剩余扇区 = %08x\r\n", APP_ADDR + i*64*1024);
		fmc_erase_sector(fmc_sector_get((uint32_t)(APP_ADDR + i*64*1024)));
    }

	while(1)
    {
        result = f_read(&file,tempbuf,sizeof(tempbuf),&bw);

		if ((result != FR_OK)||bw == 0)
		{
			printf("APP固件加载完毕\r\n");
			printf("=================================================\r\n");
			break;
		}

        /* 如果返回非0，表示编程失败 */
		TotalSize += bw;
        fmc_write_32bit_data((uint32_t)(APP_ADDR+Count*sizeof(tempbuf)),bw/4,(int32_t*)tempbuf);

        /* 显示复制进度 */
		Count = Count + 1;
		FinishPecent = (float)(TotalSize) / fno.fsize;
		printf("当前完成编程:%02d%%\r\n", (uint8_t)(FinishPecent*100));
    }
    /* 显示复制进度*/
	f_close(&file);

	/* 卸载文件系统 */
	f_mount(NULL, DiskPath, 0);
}

/*
*********************************************************************************************************
*	函 数 名: ViewRootDir
*	功能说明: 显示SD卡根目录下的文件名
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void view_root_dir(void)
{
    DIR DirInf;
    FILINFO FileInf;
    uint8_t result;
	uint8_t cnt = 0;
	

    
 	/* 挂载文件系统 */
	result = f_mount(&fs, DiskPath , 1);	/* Mount a logical drive */

	if (result != FR_OK)
	{
		printf("挂载文件系统失败 (%s)\r\n", FR_Table[result]);
	}

	/* 打开根文件夹 */
	result = f_opendir(&DirInf, DiskPath); /* 如果不带参数，则从当前目录开始 */
	if (result != FR_OK)
	{
		printf("打开根目录失败  (%s)\r\n", FR_Table[result]);
		return;
	}

	printf("属性        |  文件大小 | 短文件名 | 长文件名\r\n");
	for (cnt = 0; ;cnt++)
	{
		result = f_readdir(&DirInf, &FileInf); 		/* 读取目录项，索引会自动下移 */
		if (result != FR_OK || FileInf.fname[0] == 0)
		{
			break;
		}

		if (FileInf.fname[0] == '.')
		{
			continue;
		}

		/* 判断是文件还是子目录 */
		if (FileInf.fattrib & AM_DIR)
		{
			printf("(0x%02d)目录  ", FileInf.fattrib);
		}
		else
		{
			printf("(0x%02d)文件  ", FileInf.fattrib);
		}

		f_stat(FileInf.fname, &fno);
		
		/* 打印文件大小, 最大4G */
		printf(" %10d", (int)fno.fsize);


		printf("  %s\r\n", (char *)FileInf.fname);	/* 长文件名 */
	}

    
	/* 卸载文件系统 */
	 f_mount(NULL,DiskPath, 1);
}


