#ifndef __LCD_H__
#define __LCD_H__
#include "stm32f10x.h"
#define Bank1_LCD_C ((uint32_t)0X6C000000)  //��������ѡ���Ϊ0 ΪBANK1��4���׵�ַ A23����dc
#define Bank1_LCD_D ((uint32_t)0x6d000000)  //0x6c000000 + 0x0080 0000 *2 = 0x6d000000
#define LCD_Write_Cmd(cmd) (*((volatile uint16_t *)(Bank1_LCD_C)) = (uint16_t)(cmd))
#define LCD_Write_Data(data) (*((volatile uint16_t *)(Bank1_LCD_D)) = (uint16_t)(data))
//Һ������С
#define COLUMN 240
#define PAGE 320

#define STR_WIDTH 6
#define STR_HEIGHT 12
//��ɫ
#define WHITE		 		 0xFFFF	/* ��ɫ */
#define BLACK        0x0000	/* ��ɫ */
#define GREY         0xF7DE	/* ��ɫ */
#define BLUE         0x001F	/* ��ɫ */
#define BLUE2        0x051F	/* ǳ��ɫ */
#define RED          0xF800	/* ��ɫ */
#define MAGENTA      0xF81F	/* ����ɫ�����ɫ */
#define GREEN        0x07E0	/* ��ɫ */
#define CYAN         0x7FFF	/* ����ɫ����ɫ */
#define YELLOW       0xFFE0	/* ��ɫ */
//����ɫ
#define BACKGROUND BLACK
void LCD_GPIO_Config(void);//Һ����GPIO����
void LCD_FSMC_Config(void);//Һ����FSMC����
void LCD_Init(void);//Һ������ʼ��
void LCD_Rst(void);//Һ���������λ
void LCD_Clear(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint16_t color);
void LCD_OpenWindow(uint16_t x,uint16_t y,uint16_t width,uint16_t height);
void LCD_DrawPoint(uint16_t x,uint16_t y,uint16_t color);
void LCD_DisplayChar(uint16_t x,uint16_t y,uint8_t ch,uint16_t color);
void LCD_DisNum(uint16_t x,uint16_t y,uint16_t num,uint16_t color);
void LCD_DisStr(uint16_t x,uint16_t y,uint8_t *str,uint16_t color);
uint16_t LCD_GetPoint(uint16_t x , uint16_t y);
uint16_t LCD_RD_data(void);
//uint8_t LCD_DisplayCh(uint16_t x,uint16_t y,uint8_t * ch,uint16_t color);//��ʾһ�������ַ�
//void LCD_DisplayChStr(uint16_t x,uint16_t y,uint8_t * ch,uint16_t color);
#endif
