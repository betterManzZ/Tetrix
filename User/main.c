#include "stm32f10x.h"
#include "misc.h"
#include "bsp_led.h"
#include "ucos_ii.h"
#include "tetrix.h"
#include "lcd.h"
#include "adc.h"
#include "irDa.h"
#include "stdio.h"
#define debug 0
#if debug == 1
#include "usart1.h"
#endif
#include "irDa.h"
void delay()
{
  int i =0,j=0;
  for(i=0;i<1000;i++)
    for(j=0;j<1000;j++)
      ;
}
int main()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 
	#if debug==1
	USART1_Config();
	#endif
	LCD_Init();
  //LED_GPIO_Config();
	ADC1_Config();
	Tetrix_init();
	OS_CPU_SysTickInit();
  OSInit();
	irDa_Init();
	OSTaskCreate(&Task_tetrix_startGame,(void *)0,(OS_STK*)&Task_tetrix_mainFrame_STK[Task_tetrix_mainFrame_STK_SIZE-1],Task_tetrix_mainFrame_Prio);
  OSTaskCreate(&Task_key_deal,(void*)0,(OS_STK*)&Task_key_deal_stk[Task_key_deal_stk_size-1],Task_key_deal_prio);
	OSStart();
  while(1)
  {;
	}
		return 0;
}
