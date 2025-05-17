#ifndef __SPI_FLASH_H__
#define __SPI_FLASH_H__

#include "gd32f4xx.h"
#include <stdio.h>

#define BSP_GPIO_RCU			RCU_GPIOA
#define BSP_SPI_RCU				RCU_SPI0
#define BSP_SPI_NSS_RCU			RCU_GPIOA

#define BSP_GPIO_PORT			GPIOA
#define BSP_GPIO_AF				GPIO_AF_5

#define BSP_SPI					SPI0
#define BSP_SPI_NSS				GPIO_PIN_4
#define BSP_SPI_SCK				GPIO_PIN_5
#define BSP_SPI_MISO			GPIO_PIN_6
#define BSP_SPI_MOSI			GPIO_PIN_7

#define     W25QXX_CS_ON(x)		(gpio_bit_write(BSP_GPIO_PORT, BSP_SPI_NSS, x?1:0))
#define  	SPI_FLASH_CS_LOW()     						gpio_bit_write( BSP_GPIO_PORT, BSP_SPI_NSS , 0)
#define  	SPI_FLASH_CS_HIGH()    						gpio_bit_write( BSP_GPIO_PORT, BSP_SPI_NSS , 1)


//#define  sFLASH_ID              0xEF3015   //W25X16
//#define  sFLASH_ID              0xEF4015	 //W25Q16
//#define  sFLASH_ID              0XEF4018   //W25Q128
// #define  sFLASH_ID              0XEF4017    //W25Q64
#define  sFLASH_ID              0XEF4016    //W25Q32

#define SPI_FLASH_PageSize                  256
#define SPI_FLASH_PerWritePageSize          256
#define FLASH_SPIx                          SPI0

/*命令定义-开头*******************************/
#define W25X_WriteEnable		      0x06 
#define W25X_WriteDisable		      0x04 
#define W25X_ReadStatusReg		    0x05 
#define W25X_WriteStatusReg		    0x01 
#define W25X_ReadData			        0x03 
#define W25X_FastReadData		      0x0B 
#define W25X_FastReadDual		      0x3B 
#define W25X_PageProgram		      0x02 
#define W25X_BlockErase			      0xD8 
#define W25X_SectorErase		      0x20 
#define W25X_ChipErase			      0xC7 
#define W25X_PowerDown			      0xB9 
#define W25X_ReleasePowerDown	    0xAB 
#define W25X_DeviceID			        0xAB 
#define W25X_ManufactDeviceID   	0x90 
#define W25X_JedecDeviceID		    0x9F

/* WIP(busy)标志，FLASH内部正在写入 */
#define WIP_Flag                  0x01
#define Dummy_Byte                0xFF
/*命令定义-结尾*******************************/
/*SPI接口定义-结尾****************************/

/*等待超时时间*/
#define SPIT_FLAG_TIMEOUT         ((uint32_t)0x1000)
#define SPIT_LONG_TIMEOUT         ((uint32_t)(10 * SPIT_FLAG_TIMEOUT))

/*信息输出*/
#define FLASH_DEBUG_ON         1

#define FLASH_INFO(fmt,arg...)           printf("<<-FLASH-INFO->> "fmt"\n",##arg)
#define FLASH_ERROR(fmt,arg...)          printf("<<-FLASH-ERROR->> "fmt"\n",##arg)
#define FLASH_DEBUG(fmt,arg...)          do{\
                                          if(FLASH_DEBUG_ON)\
                                          printf("<<-FLASH-DEBUG->> [%d]"fmt"\n",__LINE__, ##arg);\
                                          }while(0)
void SPI_FLASH_Init(void);
void SPI_FLASH_SectorErase(u32 SectorAddr);
void SPI_FLASH_BulkErase(void);
void SPI_FLASH_PageWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);
void SPI_FLASH_BufferWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);
void SPI_FLASH_BufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);
u32 SPI_FLASH_ReadID(void);
u32 SPI_FLASH_ReadDeviceID(void);
void SPI_FLASH_StartReadSequence(u32 ReadAddr);
void SPI_Flash_PowerDown(void);
void SPI_Flash_WAKEUP(void);


u8 SPI_FLASH_ReadByte(void);
u8 SPI_FLASH_SendByte(u8 byte);
u16 SPI_FLASH_SendHalfWord(u16 HalfWord);
void SPI_FLASH_WriteEnable(void);
void SPI_FLASH_WaitForWriteEnd(void);

#endif


