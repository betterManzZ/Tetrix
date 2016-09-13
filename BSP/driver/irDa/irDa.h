#ifndef __irDa__h_
#define __irDa__h_
#include "stdint.h"
#include "ucos_ii.h"
/**************************************************************************************************
				irDaʹ��
**************************************************************************************************/
//[7]:���ܵ�������
//[6]:�õ�һ�ΰ�����������Ϣ
//[5]:����
//[4]:����������Ƿ񱻲���
//[3:0]:���������
extern uint8_t irSta;
extern uint32_t keyValue;
extern uint16_t timerValue;
extern uint8_t	 keyCnt;//�������´���
/**************************************************************************************************
				UCOSIIʹ��
**************************************************************************************************/
extern OS_EVENT *keyPressed;//����һ���¼������Ƿ���
extern OS_EVENT *reset;//����һ���¼�����reset�Ƿ���
#define Task_key_deal_stk_size	128u
#define Task_key_deal_prio			0
extern OS_STK Task_key_deal_stk[Task_key_deal_stk_size];
/**************************************************************************************************
				irDa��������
**************************************************************************************************/
void irDa_Init(void);
void Timer2_Config(void);
void TIM2_IRQHandler(void);
uint8_t irDa_getKeyValue(void);
void Task_key_deal(void *pData);
#endif
