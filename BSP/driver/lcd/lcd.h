#ifndef __LCD_H__
#define __LCD_H__
#include "stm32f10x.h"
#define Bank1_LCD_C ((uint32_t)0X6C000000)  //数据命令选择段为0 为BANK1第4块首地址 A23连接dc
#define Bank1_LCD_D ((uint32_t)0x6d000000)  //0x6c000000 + 0x0080 0000 *2 = 0x6d000000
#define LCD_Write_Cmd(cmd) (*((volatile uint16_t *)(Bank1_LCD_C)) = (uint16_t)(cmd))
#define LCD_Write_Data(data) (*((volatile uint16_t *)(Bank1_LCD_D)) = (uint16_t)(data))
//液晶屏大小
#define COLUMN 240
#define PAGE 320

#define STR_WIDTH 6
#define STR_HEIGHT 12
//颜色
#define WHITE		 		 0xFFFF	/* 白色 */
#define BLACK        0x0000	/* 黑色 */
#define GREY         0xF7DE	/* 灰色 */
#define BLUE         0x001F	/* 蓝色 */
#define BLUE2        0x051F	/* 浅蓝色 */
#define RED          0xF800	/* 红色 */
#define MAGENTA      0xF81F	/* 红紫色，洋红色 */
#define GREEN        0x07E0	/* 绿色 */
#define CYAN         0x7FFF	/* 蓝绿色，青色 */
#define YELLOW       0xFFE0	/* 黄色 */
//背景色
#define BACKGROUND BLACK
void LCD_GPIO_Config(void);//液晶屏GPIO配置
void LCD_FSMC_Config(void);//液晶屏FSMC配置
void LCD_Init(void);//液晶屏初始化
void LCD_Rst(void);//液晶屏软件复位
void LCD_Clear(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint16_t color);
void LCD_OpenWindow(uint16_t x,uint16_t y,uint16_t width,uint16_t height);
void LCD_DrawPoint(uint16_t x,uint16_t y,uint16_t color);
void LCD_DisplayChar(uint16_t x,uint16_t y,uint8_t ch,uint16_t color);
void LCD_DisNum(uint16_t x,uint16_t y,uint16_t num,uint16_t color);
void LCD_DisStr(uint16_t x,uint16_t y,uint8_t *str,uint16_t color);
uint16_t LCD_GetPoint(uint16_t x , uint16_t y);
uint16_t LCD_RD_data(void);
//uint8_t LCD_DisplayCh(uint16_t x,uint16_t y,uint8_t * ch,uint16_t color);//显示一个中文字符
//void LCD_DisplayChStr(uint16_t x,uint16_t y,uint8_t * ch,uint16_t color);
#endif
