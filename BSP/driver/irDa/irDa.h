#ifndef __irDa__h_
#define __irDa__h_
#include "stdint.h"
#include "ucos_ii.h"
/**************************************************************************************************
				irDa使用
**************************************************************************************************/
//[7]:接受到引导码
//[6]:得到一次按键的所有信息
//[5]:不用
//[4]:标记上升沿是否被捕获
//[3:0]:溢出计数器
extern uint8_t irSta;
extern uint32_t keyValue;
extern uint16_t timerValue;
extern uint8_t	 keyCnt;//按键按下次数
/**************************************************************************************************
				UCOSII使用
**************************************************************************************************/
extern OS_EVENT *keyPressed;//定义一个事件按键是否按下
extern OS_EVENT *reset;//定义一个事件按键reset是否按下
#define Task_key_deal_stk_size	128u
#define Task_key_deal_prio			0
extern OS_STK Task_key_deal_stk[Task_key_deal_stk_size];
/**************************************************************************************************
				irDa函数声明
**************************************************************************************************/
void irDa_Init(void);
void Timer2_Config(void);
void TIM2_IRQHandler(void);
uint8_t irDa_getKeyValue(void);
void Task_key_deal(void *pData);
#endif
