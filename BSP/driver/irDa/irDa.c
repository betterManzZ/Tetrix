#include "irDa.h"
#include "stm32f10x_tim.h"
#include "stdint.h"
#include "lcd.h"
#include "tetrix.h"
//[7]:���ܵ�������
//[6]:�õ�һ�ΰ�����������Ϣ
//[5]:����
//[4]:����������Ƿ񱻲���
//[3:0]:���������
uint8_t irSta=0;
uint32_t keyValue=0;
uint16_t timerValue=0;
uint8_t	 keyCnt=0;//�������´���
OS_EVENT *keyPressed;
OS_STK Task_key_deal_stk[Task_key_deal_stk_size];
void Timer2_Config(void)
{
	TIM_TimeBaseInitTypeDef TIM2_TIMBaseInitStructure;//��ʱ����������
	GPIO_InitTypeDef GPIO_InitStructure;//��ʱ��GPIO����
	NVIC_InitTypeDef NVIC_InitStructure;//��ʱ���ж�����
	TIM_ICInitTypeDef TIM2_ICInitStructure;//��ʱ�����벶������
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO,ENABLE);//��ΪGPIOB11��default���ܲ���TIM2_CH4�򿪶˿���ӳ�书��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	GPIO_PinRemapConfig(GPIO_FullRemap_TIM2,ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;//��������
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	TIM2_TIMBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;//����Ƶ
	TIM2_TIMBaseInitStructure.TIM_Prescaler = 72-1;							//72M/Prescaler		1us
	TIM2_TIMBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM2_TIMBaseInitStructure.TIM_Period = 10000;//�Զ���װ�ؼ���ֵ��10ms
	//TIM2_TIMBaseInitStructure.TIM_RepetitionCounter = 
	TIM_TimeBaseInit(TIM2,&TIM2_TIMBaseInitStructure);//��ʼ��TIM2��ʱ��
	
	TIM2_ICInitStructure.TIM_Channel = TIM_Channel_4;
	TIM2_ICInitStructure.TIM_ICFilter = 0x03;
	TIM2_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM2_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM2_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInit(TIM2,&TIM2_ICInitStructure);
	
	TIM_Cmd(TIM2,ENABLE);//ʹ�ܶ�ʱ��
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=2;
	NVIC_Init(&NVIC_InitStructure);
	TIM_ITConfig(TIM2,TIM_IT_Update|TIM_IT_CC4,ENABLE);	
}
void irDa_Init()
{
	keyPressed=OSSemCreate(0);//����һ���ź�������ʼֵΪ0����Ϊһ��ʼʱ��û�а������µ�
	irSta=0;
	timerValue=0;
	keyValue=0;
	Timer2_Config();
}

//�������ؿ�ʼ����
void TIM2_IRQHandler()
{
		if(TIM_GetITStatus(TIM2,TIM_IT_Update)!=RESET)//��ʱ���жϣ���ʱ�����ʱ����
		{
			if((irSta&0x80)!=0)//��40msʱ�������		˵���Ѿ���ȡ��һ������
			{			
				irSta &=~(1<<4);//��������ر������־
				if((irSta&0x0f)==0)
				{
						irSta |=(1<<6);
						OSSemPost(keyPressed);
				}
				if((irSta&0x0f)<14)
				{
					irSta++;
				}
				else
				{
					irSta&=~0X80;//���������ʾ
					irSta&=0xf0;//���������
				}
			}
		}
		if(TIM_GetITStatus(TIM2,TIM_IT_CC4)!=RESET)//�����ж�
		{
			if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11)!=0)//����������
			{
				//timerValue = TIM_GetCapture4 (TIM2);//��ȡ��ǰ����ֵ
				TIM_SetCounter(TIM2,0);//ǿ��������0
				irSta|=(1<<4);//����������Ѿ�������
				TIM_OC4PolarityConfig(TIM2,TIM_OCPolarity_Low);//���ó��½��ز���
			}
			else//�����½���
			{
				timerValue = TIM_GetCapture4(TIM2);//��ȡ������ֵ
				TIM_SetCounter(TIM2,0);
				TIM_OC4PolarityConfig(TIM2,TIM_OCPolarity_High);//���ó������ز���
				if(irSta&0x10)//���һ�θߵ�ƽ����
				{
					if(irSta&0x80)//�Ƿ��Ѿ�����������
					{
						if(timerValue>300&&timerValue<800)//��׼ֵΪ560	��560us
						{
							keyValue<<=1;
							keyValue &=~1;
						}
						else if(timerValue>1400&&timerValue<1800)//��׼ֵΪ1680  ��1680us
						{
							keyValue<<=1;
							keyValue |= 1;
						}
						
					}
					else if(timerValue>4200&&timerValue<4700)//��׼ֵΪ4500  ��4.5ms
					{
						irSta |= (1<<7);//����Ѿ�����������
						keyCnt=0;//�����������
					}
					else if(timerValue>2200&&timerValue<2600)
					{
						irSta &= ~((1<<4)&(1<<7));
						keyCnt++;
					}
					irSta &= ~(1<<4);//�������ر�־λ��0
				}		
			}
		}
		TIM_ClearFlag(TIM2,TIM_FLAG_CC4|TIM_IT_Update);//���ж������־λ
}
/*��ȡ����ֵ*/
uint8_t irDa_getKeyValue(void)
{        
	uint8_t sta=0;       
  uint8_t t1,t2;  
	if(irSta&(1<<6))
	{ 
	    t1=keyValue>>24;			
	    t2=(keyValue>>16)&0xff;	
 	    if((t1==(u8)~t2)&&t1==0)
	    { 
	        t1=keyValue>>8;
	        t2=keyValue; 	
	        if(t1==(u8)~t2)sta=t1;
			}   
		irSta&=~(1<<6);
		keyCnt=0;
	}  
    return sta;
}
/*������������*/
void Task_key_deal(void *pData)
{
	uint8_t error=0;
	uint8_t value=0;
	while(1)
	{
		value=0;
		OSSemPend(keyPressed,0,&error);//�ȴ��ź���
		value=irDa_getKeyValue();
		/*�����Ƕ԰����ĸ��ִ������*/
		if((gameState&GAME_Pause_Mask)!=0&&(gameState&GAME_Over_Mask)!=0)//��ǰ״̬��Ϊ��ͣ״̬����Ϸ��Ϊ����״̬
		{
			switch(value)
			{
				case 0:
					break;
				case 2://up
					Tetrix_ChangeState();
					break;
				case 224://left
					Tetrix_moveLeft();
					break;
				case 144://right
					Tetrix_moveRight();
					break;
				case 152://down
					Tetrix_moveDown();
					break;
				case 168://pause
					OSTmrStop(os_tmr1,OS_TMR_OPT_NONE,(void*)0,(INT8U*)&error);
					gameState&=~GAME_Pause_Mask;
					break;
				case 194://return
					//���¿�ʼ��Ϸ
					break;
				default:
					break;
			}
		}
		//�����Ϸ����,��up down left right pause 5������������
		else if((gameState&GAME_Over_Mask)==0)
		{
			if(value==194)//�����ֵΪreset,�����¿�ʼ��Ϸ
			{
				
			}
			
		}
		else if((gameState&GAME_Pause_Mask)==0 && (gameState&GAME_Over_Mask)!=0)//�����ǰ��ϷΪ��ͣ״̬ 
		{
				if(value==168)//�����ֵΪpause 
				{
					OSTmrStart(os_tmr1,(uint8_t*)&error);//�������������ʱ��
					gameState|=GAME_Pause_Mask;//����Ϸ״̬���λ����ͣ״̬
				}
				else if(value==194)//�����ֵΪreset,�����¿�ʼ��Ϸ
				{
					
				}
		}
	}
}

