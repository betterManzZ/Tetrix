#include "lcd.h"
//#include "USART1.h"
#include "stm32f10x_fsmc.h"
#include "asc.h"
//#include "ff.h"
void LCD_Delay(uint32_t time)
{
	while(time--)
	{
		
	}
}
void LCD_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOG,ENABLE);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//reset
	GPIO_Init(GPIOG,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1
																| GPIO_Pin_4 | GPIO_Pin_5
																| GPIO_Pin_14 | GPIO_Pin_15
																| GPIO_Pin_8 | GPIO_Pin_9
																| GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_Init(GPIOD,&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 |GPIO_Pin_9 |GPIO_Pin_7
																| GPIO_Pin_10 | GPIO_Pin_11
																| GPIO_Pin_12 | GPIO_Pin_13
																| GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_Init(GPIOG,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;//屏幕背光
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	GPIO_ResetBits(GPIOB,GPIO_Pin_1);
	
}
void LCD_FSMC_Config(void)
{
	FSMC_NORSRAMTimingInitTypeDef FSMC_NORSRAMTimingInitStructure;
	FSMC_NORSRAMInitTypeDef FSMC_NORSRAMInitStructure;
	//根据datasheet 计算
	FSMC_NORSRAMTimingInitStructure.FSMC_AccessMode = FSMC_AccessMode_B;//MODE B
	FSMC_NORSRAMTimingInitStructure.FSMC_AddressHoldTime = 0x00;
	FSMC_NORSRAMTimingInitStructure.FSMC_AddressSetupTime = 0x02;
	FSMC_NORSRAMTimingInitStructure.FSMC_BusTurnAroundDuration = 0x00;//总线恢复时间
	FSMC_NORSRAMTimingInitStructure.FSMC_CLKDivision = 0x00;//时钟分频
	FSMC_NORSRAMTimingInitStructure.FSMC_DataLatency = 0x00;
	FSMC_NORSRAMTimingInitStructure.FSMC_DataSetupTime = 0x05;//数据建立保持时间
	
	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;
	FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM4;
	FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;//地址线和数据线不复用
	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;//禁止扩展模式，读写时序相同
	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
	FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_NOR;
	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &FSMC_NORSRAMTimingInitStructure;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;//当前模式只在突发模式时有效
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;//当前模式只在突发模式时有效
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;//当前模式只在突发模式时有效
	FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;//禁止突发写
	FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &FSMC_NORSRAMTimingInitStructure;
	
  FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);
	FSMC_NORSRAMCmd (FSMC_Bank1_NORSRAM4,ENABLE);
}
void LCD_Rst(void)//软件复位
{
	GPIO_ResetBits(GPIOG,GPIO_Pin_11);
	LCD_Delay(0xfffff);
	GPIO_SetBits(GPIOG,GPIO_Pin_11);
	LCD_Delay(0xfffff);
}
void LCD_REG_Config(void)
{
	// Power control B (CFh)  功耗控制B*/
	LCD_Delay(1);
	LCD_Write_Cmd(0xCF);
	LCD_Write_Data(0x00);
	LCD_Write_Data(0x81);
	LCD_Write_Data(0x30);
	
	/*  Power on sequence control (EDh)电源时序控制 */
	LCD_Delay(1);
	LCD_Write_Cmd(0xED);
	LCD_Write_Data(0x64);
	LCD_Write_Data(0x03);
	LCD_Write_Data(0x12);
	LCD_Write_Data(0x81);
	
	/*  Driver timing control A (E8h) 驱动时序控制A */
	LCD_Delay(1);
	LCD_Write_Cmd(0xE8);
	LCD_Write_Data(0x85);
	LCD_Write_Data(0x10);
	LCD_Write_Data(0x78);
	
	/*  Power control A (CBh) 功耗控制A*/
	LCD_Delay(1);
	LCD_Write_Cmd(0xCB);
	LCD_Write_Data(0x39);
	LCD_Write_Data(0x2C);
	LCD_Write_Data(0x00);
	LCD_Write_Data(0x34);
	LCD_Write_Data(0x02);
	
	/* Pump ratio control (F7h) 泵比设置*/
	LCD_Delay(1);
	LCD_Write_Cmd(0xF7);
	LCD_Write_Data(0x20);
	
	/* Driver timing control B 驱动时序控制B*/
	LCD_Delay(1);
	LCD_Write_Cmd(0xEA);
	LCD_Write_Data(0x00);
	LCD_Write_Data(0x00);
	
	/* Frame Rate Control (In Normal Mode/Full Colors) (B1h) 帧速率控制（在正常模式/全色模式）*/
	LCD_Delay(1);
	LCD_Write_Cmd(0xB1);
	LCD_Write_Data(0x00);//fosc
	LCD_Write_Data(0x1B);//默认帧速率70HZ
	
	/*  Display Function Control (B6h) 显示功能设置*/
	LCD_Delay(1);
	LCD_Write_Cmd(0xB6);
	LCD_Write_Data(0x0A);
	LCD_Write_Data(0xA2);
	
	/* Power Control 1 (C0h) 功耗控制1 设置GVDD电平*/
	LCD_Delay(1);
	LCD_Write_Cmd(0xC0);
	LCD_Write_Data(0x35);
	
	/* Power Control 2 (C1h) 功耗控制2*/
	LCD_Delay(1);
	LCD_Write_Cmd(0xC1);
	LCD_Write_Data(0x11);
	
	/* VCOM Control 1(C5h) VCOM控制1 设置VCOMH和VCOML电压 */
	LCD_Write_Cmd(0xC5);
	LCD_Write_Data(0x45);
	LCD_Write_Data(0x45);
	
	/*  VCOM Control 2(C7h) VCOM控制2  */
	LCD_Write_Cmd(0xC7);
	LCD_Write_Data(0xA2);
	
	/* Enable 3G (F2h) 使能3G */
	LCD_Write_Cmd(0xF2);
	LCD_Write_Data(0x00);
	
	/* Gamma Set (26h)  伽马设置 */
	LCD_Write_Cmd(0x26);
	LCD_Write_Data(0x01);//选择伽马曲线1
	LCD_Delay(1);
	
	/* Positive Gamma Correction 正级伽马校准 设置灰度电压调整TFT面板伽马特性*/
	LCD_Write_Cmd(0xE0); //Set Gamma
	LCD_Write_Data(0x0F);
	LCD_Write_Data(0x26);
	LCD_Write_Data(0x24);
	LCD_Write_Data(0x0B);
	LCD_Write_Data(0x0E);
	LCD_Write_Data(0x09);
	LCD_Write_Data(0x54);
	LCD_Write_Data(0xA8);
	LCD_Write_Data(0x46);
	LCD_Write_Data(0x0C);
	LCD_Write_Data(0x17);
	LCD_Write_Data(0x09);
	LCD_Write_Data(0x0F);
	LCD_Write_Data(0x07);
	LCD_Write_Data(0x00);
	
	/* Negative Gamma Correction (E1h) 负极伽马校准 */
	LCD_Write_Cmd(0XE1); //Set Gamma
	LCD_Write_Data(0x00);
	LCD_Write_Data(0x19);
	LCD_Write_Data(0x1B);
	LCD_Write_Data(0x04);
	LCD_Write_Data(0x10);
	LCD_Write_Data(0x07);
	LCD_Write_Data(0x2A);
	LCD_Write_Data(0x47);
	LCD_Write_Data(0x39);
	LCD_Write_Data(0x03);
	LCD_Write_Data(0x06);
	LCD_Write_Data(0x06);
	LCD_Write_Data(0x30);
	LCD_Write_Data(0x38);
	LCD_Write_Data(0x0F);
	
	//存储器访问控制
	LCD_Delay(1);
	LCD_Write_Cmd(0X36);
	LCD_Write_Data(0xc8);//选择竖屏，从左上角到右下角
	
	//列地址设置 column address control set
	LCD_Delay(1);
	LCD_Write_Cmd(0X2A);
	LCD_Write_Data(0x00);//列起始地址
	LCD_Write_Data(0x00);
	LCD_Write_Data(0xef);//列结束地址，高字节在前
	LCD_Write_Data(0x00);
	
	//page设置 page address control set
	LCD_Delay(1);
	LCD_Write_Cmd(0X2B);
	LCD_Write_Data(0x00);//page起始地址
	LCD_Write_Data(0x00);
	LCD_Write_Data(0x3f);//page结束地址319 高位在前
	LCD_Write_Data(0x01);
	
	//像素格式设置 COLMODE SET
	LCD_Delay(1);
	LCD_Write_Cmd(0X3A);
	LCD_Write_Data(0x55);//设置为16bit/像素
	
	//推出睡眠模式 SLPOUT
	LCD_Write_Cmd(0X11);
	LCD_Delay(0xAfff<<2);//需等待5ms
	
	//开显示 DISPON
	LCD_Write_Cmd(0X29);
	LCD_Delay(1);

}
void LCD_Init(void)
{
	LCD_GPIO_Config();
	LCD_FSMC_Config();
	LCD_Rst();
	LCD_REG_Config();
}
void LCD_Clear(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint16_t color)
{
	uint32_t i=0;
	//设置行坐标
	LCD_Write_Cmd(0x2a);
	LCD_Write_Data(x>>8);
	LCD_Write_Data(x&0xff);
	LCD_Write_Data((width+x-1)>>8);
	LCD_Write_Data((width+x-1)&0xff);
	//设置列坐标
	LCD_Write_Cmd(0x2b);
	LCD_Write_Data(y>>8);
	LCD_Write_Data(y&0xff);
	LCD_Write_Data((y+height-1)>>8);
	LCD_Write_Data((y+height-1)&0xff);
	//写数据
	LCD_Write_Cmd(0x2c);
	for(i = 0;i<width*height;i++)
	{
		LCD_Write_Data(color);
	}
}
void LCD_OpenWindow(uint16_t x,uint16_t y,uint16_t width,uint16_t height)
{
	//设置行坐标
	LCD_Write_Cmd(0x2a);
	LCD_Write_Data(x>>8);
	LCD_Write_Data(x&0xff);
	LCD_Write_Data((width+x-1)>>8); 
	LCD_Write_Data((width+x-1)&0xff);
	//设置列坐标
	LCD_Write_Cmd(0x2b);
	LCD_Write_Data(y>>8);
	LCD_Write_Data(y&0xff);
	LCD_Write_Data((y+height-1)>>8);
	LCD_Write_Data((y+height-1)&0xff);
}
void LCD_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
{
	LCD_OpenWindow(x,y,0,0);
	LCD_Write_Cmd(0x2c);
	LCD_Write_Data(color);
}
void LCD_DisplayChar(uint16_t x,uint16_t y,uint8_t ch,uint16_t color)//显示一个字符
{
	uint16_t index;
	uint16_t i = 0;
	uint16_t j = 0;
	uint8_t temp;
	index = ch-' ';
	LCD_OpenWindow(x,y,6,12);
	LCD_Write_Cmd(0x2c);
	for(i = 0;i<12;i++)
	{
		temp = asc1_1206[index][i];
		for(j = 0;j<6;j++)
		{
			if(temp&0x01)
			{
				LCD_Write_Data(color);
			}
			else
			{
				LCD_Write_Data(BACKGROUND);
			}
			temp >>=1;
		}
	}
}
void LCD_DisStr(uint16_t x,uint16_t y,uint8_t *str,uint16_t color)
{
	while(*str!='\0')
	{
		if(x>(COLUMN-STR_WIDTH))
		{
			x = 0;
			y+=STR_HEIGHT;
		}
		if(y>(PAGE-STR_HEIGHT))
		{
			y =0;
			x = 0;
		}
		LCD_DisplayChar(x,y,*str,color);
		str++;  
		x+=STR_WIDTH;
	}
}
void LCD_DisNum(uint16_t x,uint16_t y,uint16_t num,uint16_t color)
{
	uint16_t temp,length=0;
	temp = num;
	if(num==0)
	{
		LCD_DisplayChar(x,y,'0',color);
		return ;
	}
	while(temp)
	{
		temp=temp/10;
		length++;
	}
	while(length)
	{
		LCD_DisplayChar(x+STR_WIDTH*(length--),y,(num%10)+'0',color);
		num = num/10;
	}
	
}
/*读数据*/
uint16_t LCD_RD_data()
{
	uint16_t R=0,G=0,B=0;
	R = *(__IO uint16_t*)Bank1_LCD_D;
	R = *(__IO uint16_t*)Bank1_LCD_D;
	G = *(__IO uint16_t*)Bank1_LCD_D;
	B = *(__IO uint16_t*)Bank1_LCD_D;
	return (((R>>11)<<11) | ((G>>10)<<5) | (B>>11));
}
/*读取LCD中点的颜色*/
uint16_t LCD_GetPoint(uint16_t x , uint16_t y)
{
	LCD_OpenWindow(x, y,1,1);
	LCD_Write_Cmd(0x2e);         /* 读数据 */
	return (LCD_RD_data());
}
/*uint8_t LCD_DisplayCh(uint16_t x,uint16_t y,uint8_t * ch,uint16_t color)//显示一个中文字符
{
	uint8_t buffer[32];
	uint8_t h = *ch;//汉字的区码
	uint8_t l = *(ch+1);//汉字的位码
	int index = ((h-0xb0)*94 + l-0xa1)*32;//汉字在字库中的位置 数据类型使用int型
	FRESULT result = FR_OK;
	FATFS fs;
	FIL sfile;
	UINT rdnum;
	uint16_t i;
	uint16_t j;
	uint16_t temp;
	result = f_mount(0,&fs);//开辟一个磁盘空间
	if(result == FR_OK)
	{
		result = f_open(&sfile,"0:/HZLIB.bin",FA_OPEN_EXISTING | FA_READ);
	}
	else
	{
		return 0;
	}
	if(result == FR_OK)
	{
		f_lseek(&sfile,index);
		result = f_read(&sfile,buffer,32,&rdnum);
		f_close(&sfile);
	}
	else
	{
		return 0;
	}
	LCD_OpenWindow(x,y,16,16);
	LCD_Write_Cmd(0x2c);
	for(i = 0;i<16;i++)
	{
		temp = buffer[i*2];
		temp = (temp<<8) | buffer[i*2+1];
		for(j = 0;j<16;j++)
		{
			if(temp&0x8000)
			{
				LCD_Write_Data(color);
			}
			else
			{
				LCD_Write_Data(BACKGROUND);
			}
			temp <<= 1;
		}
	}
	return 1;
}
void LCD_DisplayChStr(uint16_t x,uint16_t y,uint8_t * ch,uint16_t color)
{
	uint16_t i = 0;
	while(*ch!='\0')
	{
		LCD_DisplayCh(x+16*i,y,ch,color);
		i++;
		ch+=2;
	}
}
*/