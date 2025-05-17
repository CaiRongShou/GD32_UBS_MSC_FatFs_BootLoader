#include "spi_flash.h"

/**********************************************************
 * 函 数 名 称：bsp_spi_init
 * 函 数 功 能：初始化SPI
 * 传 入 参 数：无
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
**********************************************************/
void SPI_FLASH_Init(void)
{
    rcu_periph_clock_enable(BSP_GPIO_RCU);    // 使用A端口
    rcu_periph_clock_enable(BSP_SPI_RCU);     // 使能SPI0

    //引脚复用
    gpio_af_set(BSP_GPIO_PORT, GPIO_AF_5, BSP_SPI_SCK);
    gpio_af_set(BSP_GPIO_PORT, GPIO_AF_5, BSP_SPI_MISO);
    gpio_af_set(BSP_GPIO_PORT, GPIO_AF_5, BSP_SPI_MOSI);
    //引脚模式
    gpio_mode_set(BSP_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, BSP_SPI_SCK);
    gpio_mode_set(BSP_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, BSP_SPI_MISO);
    gpio_mode_set(BSP_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, BSP_SPI_MOSI);
    //输出模式
    gpio_output_options_set(BSP_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_SPI_SCK);
    gpio_output_options_set(BSP_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_SPI_MISO);
    gpio_output_options_set(BSP_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_SPI_MOSI);

    //开启CS引脚时钟
    rcu_periph_clock_enable(BSP_SPI_NSS_RCU);
    //配置CS引脚模式
    gpio_mode_set(BSP_GPIO_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, BSP_SPI_NSS);
    //配置CS输出模式
    gpio_output_options_set(BSP_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_SPI_NSS);
    //GD25Q32不选中
    gpio_bit_write(BSP_GPIO_PORT, BSP_SPI_NSS, SET);

    //SPI参数定义结构体
    spi_parameter_struct spi_init_struct;
    spi_init_struct.trans_mode        		= SPI_TRANSMODE_FULLDUPLEX;  // 传输模式全双工
    spi_init_struct.device_mode       		= SPI_MASTER;                // 配置为主机
    spi_init_struct.frame_size         		= SPI_FRAMESIZE_8BIT;        // 8位数据
    spi_init_struct.clock_polarity_phase 	= SPI_CK_PL_HIGH_PH_2EDGE;   // 极性相位
    spi_init_struct.nss               		= SPI_NSS_SOFT;              // 软件cs
    spi_init_struct.prescale           		= SPI_PSC_4;                 // SPI时钟预调因数为4
    spi_init_struct.endian            		= SPI_ENDIAN_MSB;            // 高位在前
    //将参数填入SPI0
    spi_init(BSP_SPI, &spi_init_struct);
    //使能SPI
    spi_enable(BSP_SPI);
}
 /**
  * @brief  擦除FLASH扇区
  * @param  SectorAddr：要擦除的扇区地址
  * @retval 无
  */
 void SPI_FLASH_SectorErase(u32 SectorAddr)
 {
   /* 发送FLASH写使能命令 */
   SPI_FLASH_WriteEnable();
   SPI_FLASH_WaitForWriteEnd();
   /* 擦除扇区 */
   /* 选择FLASH: CS低电平 */
   SPI_FLASH_CS_LOW();
   /* 发送扇区擦除指令*/
   SPI_FLASH_SendByte(W25X_SectorErase);
   /*发送擦除扇区地址的高位*/
   SPI_FLASH_SendByte((SectorAddr & 0xFF0000) >> 16);
   /* 发送擦除扇区地址的中位 */
   SPI_FLASH_SendByte((SectorAddr & 0xFF00) >> 8);
   /* 发送擦除扇区地址的低位 */
   SPI_FLASH_SendByte(SectorAddr & 0xFF);
   /* 停止信号 FLASH: CS 高电平 */
   SPI_FLASH_CS_HIGH();
   /* 等待擦除完毕*/
   SPI_FLASH_WaitForWriteEnd();
 }
 
  /**
   * @brief  擦除FLASH扇区，整片擦除
   * @param  无
   * @retval 无
   */
 void SPI_FLASH_BulkErase(void)
 {
   /* 发送FLASH写使能命令 */
   SPI_FLASH_WriteEnable();
 
   /* 整块 Erase */
   /* 选择FLASH: CS低电平 */
   SPI_FLASH_CS_LOW();
   /* 发送整块擦除指令*/
   SPI_FLASH_SendByte(W25X_ChipErase);
   /* 停止信号 FLASH: CS 高电平 */
   SPI_FLASH_CS_HIGH();
 
   /* 等待擦除完毕*/
   SPI_FLASH_WaitForWriteEnd();
 }
 
  /**
   * @brief  对FLASH按页写入数据，调用本函数写入数据前需要先擦除扇区
   * @param	pBuffer，要写入数据的指针
   * @param WriteAddr，写入地址
   * @param  NumByteToWrite，写入数据长度，必须小于等于SPI_FLASH_PerWritePageSize
   * @retval 无
   */
 void SPI_FLASH_PageWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
 {
    
   /* 发送FLASH写使能命令 */
   SPI_FLASH_WriteEnable();
 
   /* 选择FLASH: CS低电平 */
   SPI_FLASH_CS_LOW();
   /* 写页写指令*/
   SPI_FLASH_SendByte(W25X_PageProgram);
   /*发送写地址的高位*/
   SPI_FLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
   /*发送写地址的中位*/
   SPI_FLASH_SendByte((WriteAddr & 0xFF00) >> 8);
   /*发送写地址的低位*/
   SPI_FLASH_SendByte(WriteAddr & 0xFF);
 
   if(NumByteToWrite > SPI_FLASH_PerWritePageSize)
   {
      NumByteToWrite = SPI_FLASH_PerWritePageSize;
      FLASH_ERROR("SPI_FLASH_PageWrite too large!"); 
   }
 
   /* 写入数据*/
   while (NumByteToWrite--)
   {
     /* 发送当前要写入的字节数据 */
     SPI_FLASH_SendByte(*pBuffer);
     /* 指向下一字节数据 */
     pBuffer++;
   }
 
   /* 停止信号 FLASH: CS 高电平 */
   SPI_FLASH_CS_HIGH();
 
   /* 等待写入完毕*/
   SPI_FLASH_WaitForWriteEnd();
 }
 
  /**
   * @brief  对FLASH写入数据，调用本函数写入数据前需要先擦除扇区
   * @param	pBuffer，要写入数据的指针
   * @param  WriteAddr，写入地址
   * @param  NumByteToWrite，写入数据长度
   * @retval 无
   */
 void SPI_FLASH_BufferWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
 {
   u8 NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;
     
     /*mod运算求余，若writeAddr是SPI_FLASH_PageSize整数倍，运算结果Addr值为0*/
   Addr = WriteAddr % SPI_FLASH_PageSize;
     
     /*差count个数据值，刚好可以对齐到页地址*/
   count = SPI_FLASH_PageSize - Addr;
     /*计算出要写多少整数页*/
   NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
     /*mod运算求余，计算出剩余不满一页的字节数*/
   NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;
     
     /* Addr=0,则WriteAddr 刚好按页对齐 aligned  */
   if (Addr == 0)
   {
         /* NumByteToWrite < SPI_FLASH_PageSize */
     if (NumOfPage == 0) 
     {
       SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
     }
     else /* NumByteToWrite > SPI_FLASH_PageSize */
     { 
             /*先把整数页都写了*/
       while (NumOfPage--)
       {
         SPI_FLASH_PageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
         WriteAddr +=  SPI_FLASH_PageSize;
         pBuffer += SPI_FLASH_PageSize;
       }
             /*若有多余的不满一页的数据，把它写完*/
       SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
     }
   }
     /* 若地址与 SPI_FLASH_PageSize 不对齐  */
   else 
   {
         /* NumByteToWrite < SPI_FLASH_PageSize */
     if (NumOfPage == 0)
     {
             /*当前页剩余的count个位置比NumOfSingle小，一页写不完*/
       if (NumOfSingle > count) 
       {
         temp = NumOfSingle - count;
                 /*先写满当前页*/
         SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
                 
         WriteAddr +=  count;
         pBuffer += count;
                 /*再写剩余的数据*/
         SPI_FLASH_PageWrite(pBuffer, WriteAddr, temp);
       }
       else /*当前页剩余的count个位置能写完NumOfSingle个数据*/
       {
         SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
       }
     }
     else /* NumByteToWrite > SPI_FLASH_PageSize */
     {
             /*地址不对齐多出的count分开处理，不加入这个运算*/
       NumByteToWrite -= count;
       NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
       NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;
             
             /* 先写完count个数据，为的是让下一次要写的地址对齐 */
       SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
             
             /* 接下来就重复地址对齐的情况 */
       WriteAddr +=  count;
       pBuffer += count;
             /*把整数页都写了*/
       while (NumOfPage--)
       {
         SPI_FLASH_PageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
         WriteAddr +=  SPI_FLASH_PageSize;
         pBuffer += SPI_FLASH_PageSize;
       }
             /*若有多余的不满一页的数据，把它写完*/
       if (NumOfSingle != 0)
       {
         SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
       }
     }
   }
 }
 
  /**
   * @brief  读取FLASH数据
   * @param 	pBuffer，存储读出数据的指针
   * @param   ReadAddr，读取地址
   * @param   NumByteToRead，读取数据长度
   * @retval 无
   */
 void SPI_FLASH_BufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
 {
   /* 选择FLASH: CS低电平 */
   SPI_FLASH_CS_LOW();
 
   /* 发送 读 指令 */
   SPI_FLASH_SendByte(W25X_ReadData);
 
   /* 发送 读 地址高位 */
   SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
   /* 发送 读 地址中位 */
   SPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
   /* 发送 读 地址低位 */
   SPI_FLASH_SendByte(ReadAddr & 0xFF);
     
     /* 读取数据 */
   while (NumByteToRead--) /* while there is data to be read */
   {
     /* 读取一个字节*/
     *pBuffer = SPI_FLASH_SendByte(Dummy_Byte);
     /* 指向下一个字节缓冲区 */
     pBuffer++;
   }
 
   /* 停止信号 FLASH: CS 高电平 */
   SPI_FLASH_CS_HIGH();
 }
 
  /**
   * @brief  读取FLASH ID
   * @param 	无
   * @retval FLASH ID
   */
 u32 SPI_FLASH_ReadID(void)
 {
   u32 Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;
 
   /* 开始通讯：CS低电平 */
   SPI_FLASH_CS_LOW();
 
   /* 发送JEDEC指令，读取ID */
   SPI_FLASH_SendByte(W25X_JedecDeviceID);
 
   /* 读取一个字节数据 */
   Temp0 = SPI_FLASH_SendByte(Dummy_Byte);
 
   /* 读取一个字节数据 */
   Temp1 = SPI_FLASH_SendByte(Dummy_Byte);
 
   /* 读取一个字节数据 */
   Temp2 = SPI_FLASH_SendByte(Dummy_Byte);
 
  /* 停止通讯：CS高电平 */
   SPI_FLASH_CS_HIGH();
 
   /*把数据组合起来，作为函数的返回值*/
     Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;
 
   return Temp;
 }
  /**
   * @brief  读取FLASH Device ID
   * @param 	无
   * @retval FLASH Device ID
   */
 u32 SPI_FLASH_ReadDeviceID(void)
 {
   u32 Temp = 0;
 
   /* Select the FLASH: Chip Select low */
   SPI_FLASH_CS_LOW();
 
   /* Send "RDID " instruction */
   SPI_FLASH_SendByte(W25X_DeviceID);
   SPI_FLASH_SendByte(Dummy_Byte);
   SPI_FLASH_SendByte(Dummy_Byte);
   SPI_FLASH_SendByte(Dummy_Byte);
   
   /* Read a byte from the FLASH */
   Temp = SPI_FLASH_SendByte(Dummy_Byte);
 
   /* Deselect the FLASH: Chip Select high */
   SPI_FLASH_CS_HIGH();
 
   return Temp;
 }
 /*******************************************************************************
 * Function Name  : SPI_FLASH_StartReadSequence
 * Description    : Initiates a read data byte (READ) sequence from the Flash.
 *                  This is done by driving the /CS line low to select the device,
 *                  then the READ instruction is transmitted followed by 3 bytes
 *                  address. This function exit and keep the /CS line low, so the
 *                  Flash still being selected. With this technique the whole
 *                  content of the Flash is read with a single READ instruction.
 * Input          : - ReadAddr : FLASH's internal address to read from.
 * Output         : None
 * Return         : None
 *******************************************************************************/
 void SPI_FLASH_StartReadSequence(u32 ReadAddr)
 {
   /* Select the FLASH: Chip Select low */
   SPI_FLASH_CS_LOW();
 
   /* Send "Read from Memory " instruction */
   SPI_FLASH_SendByte(W25X_ReadData);
 
   /* Send the 24-bit address of the address to read from -----------------------*/
   /* Send ReadAddr high nibble address byte */
   SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
   /* Send ReadAddr medium nibble address byte */
   SPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
   /* Send ReadAddr low nibble address byte */
   SPI_FLASH_SendByte(ReadAddr & 0xFF);
 }
 
 
  /**
   * @brief  使用SPI读取一个字节的数据
   * @param  无
   * @retval 返回接收到的数据
   */
 u8 SPI_FLASH_ReadByte(void)
 {
   return (SPI_FLASH_SendByte(Dummy_Byte));
 }
 
  /**
   * @brief  使用SPI发送一个字节的数据
   * @param  byte：要发送的数据
   * @retval 返回接收到的数据
   */
 u8 SPI_FLASH_SendByte(u8 byte)
 {
    //等待发送缓冲区为空
    while(RESET == spi_i2s_flag_get(FLASH_SPIx,  SPI_FLAG_TBE) );
    //通过SPI4发送一个字节数据
    spi_i2s_data_transmit(FLASH_SPIx, byte);

    //等待接收缓冲区不空标志
    while(RESET == spi_i2s_flag_get(FLASH_SPIx,  SPI_FLAG_RBNE) );
    //读取并返回在SPI4读取到的单字节数据
    return spi_i2s_data_receive(FLASH_SPIx);
 }
   /**
   * @brief  向FLASH发送 写使能 命令
   * @param  none
   * @retval none
   */
 void SPI_FLASH_WriteEnable(void)
 {
   /* 通讯开始：CS低 */
   SPI_FLASH_CS_LOW();
 
   /* 发送写使能命令*/
   SPI_FLASH_SendByte(W25X_WriteEnable);
 
   /*通讯结束：CS高 */
   SPI_FLASH_CS_HIGH();
 }
 
 /* WIP(busy)标志，FLASH内部正在写入 */
 #define WIP_Flag                  0x01
 
  /**
   * @brief  等待WIP(BUSY)标志被置0，即等待到FLASH内部数据写入完毕
   * @param  none
   * @retval none
   */
 void SPI_FLASH_WaitForWriteEnd(void)
 {
   u8 FLASH_Status = 0;
 
   /* 选择 FLASH: CS 低 */
   SPI_FLASH_CS_LOW();
 
   /* 发送 读状态寄存器 命令 */
   SPI_FLASH_SendByte(W25X_ReadStatusReg);
 
   /* 若FLASH忙碌，则等待 */
   do
   {
         /* 读取FLASH芯片的状态寄存器 */
     FLASH_Status = SPI_FLASH_SendByte(Dummy_Byte);	 
   }
   while ((FLASH_Status & WIP_Flag) == SET);  /* 正在写入标志 */
 
   /* 停止信号  FLASH: CS 高 */
   SPI_FLASH_CS_HIGH();
 }
 
 
 //进入掉电模式
 void SPI_Flash_PowerDown(void)   
 { 
   /* 通讯开始：CS低 */
   SPI_FLASH_CS_LOW();
 
   /* 发送 掉电 命令 */
   SPI_FLASH_SendByte(W25X_PowerDown);
 
   /*通讯结束：CS高 */
   SPI_FLASH_CS_HIGH();
 }   
 
 //唤醒
 void SPI_Flash_WAKEUP(void)   
 {
   /*选择 FLASH: CS 低 */
   SPI_FLASH_CS_LOW();
 
   /* 发送 上电 命令 */
   SPI_FLASH_SendByte(W25X_ReleasePowerDown);
 
    /* 停止信号 FLASH: CS 高 */
   SPI_FLASH_CS_HIGH();
 }   
    
     
 /*********************************************END OF FILE**********************/
