#include "irDa.h"
#include "stm32f10x_tim.h"
#include "stdint.h"
#include "lcd.h"
#include "tetrix.h"
//[7]:接受到引导码
//[6]:得到一次按键的所有信息
//[5]:不用
//[4]:标记上升沿是否被捕获
//[3:0]:溢出计数器
uint8_t irSta=0;
uint32_t keyValue=0;
uint16_t timerValue=0;
uint8_t	 keyCnt=0;//按键按下次数
OS_EVENT *keyPressed;
OS_STK Task_key_deal_stk[Task_key_deal_stk_size];
void Timer2_Config(void)
{
	TIM_TimeBaseInitTypeDef TIM2_TIMBaseInitStructure;//定时器基本配置
	GPIO_InitTypeDef GPIO_InitStructure;//定时器GPIO配置
	NVIC_InitTypeDef NVIC_InitStructure;//定时器中断配置
	TIM_ICInitTypeDef TIM2_ICInitStructure;//定时器输入捕获配置
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO,ENABLE);//因为GPIOB11的default功能不是TIM2_CH4打开端口重映射功能
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	GPIO_PinRemapConfig(GPIO_FullRemap_TIM2,ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;//上拉输入
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	TIM2_TIMBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;//不分频
	TIM2_TIMBaseInitStructure.TIM_Prescaler = 72-1;							//72M/Prescaler		1us
	TIM2_TIMBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM2_TIMBaseInitStructure.TIM_Period = 10000;//自动重装载计数值，10ms
	//TIM2_TIMBaseInitStructure.TIM_RepetitionCounter = 
	TIM_TimeBaseInit(TIM2,&TIM2_TIMBaseInitStructure);//初始化TIM2定时器
	
	TIM2_ICInitStructure.TIM_Channel = TIM_Channel_4;
	TIM2_ICInitStructure.TIM_ICFilter = 0x03;
	TIM2_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM2_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM2_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInit(TIM2,&TIM2_ICInitStructure);
	
	TIM_Cmd(TIM2,ENABLE);//使能定时器
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=2;
	NVIC_Init(&NVIC_InitStructure);
	TIM_ITConfig(TIM2,TIM_IT_Update|TIM_IT_CC4,ENABLE);	
}
void irDa_Init()
{
	keyPressed=OSSemCreate(0);//创建一个信号量，初始值为0，因为一开始时是没有按键按下的
	irSta=0;
	timerValue=0;
	keyValue=0;
	Timer2_Config();
}

//从上升沿开始捕获
void TIM2_IRQHandler()
{
		if(TIM_GetITStatus(TIM2,TIM_IT_Update)!=RESET)//定时器中断，定时器溢出时产生
		{
			if((irSta&0x80)!=0)//在40ms时发生溢出		说明已经获取到一次数据
			{			
				irSta &=~(1<<4);//清除上升沿被捕获标志
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
					irSta&=~0X80;//清除引导标示
					irSta&=0xf0;//清除计数器
				}
			}
		}
		if(TIM_GetITStatus(TIM2,TIM_IT_CC4)!=RESET)//捕获中断
		{
			if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11)!=0)//捕获到上升沿
			{
				//timerValue = TIM_GetCapture4 (TIM2);//读取当前计数值
				TIM_SetCounter(TIM2,0);//强计数器清0
				irSta|=(1<<4);//标记上升沿已经被捕获
				TIM_OC4PolarityConfig(TIM2,TIM_OCPolarity_Low);//设置成下降沿捕获
			}
			else//捕获到下降沿
			{
				timerValue = TIM_GetCapture4(TIM2);//获取计数器值
				TIM_SetCounter(TIM2,0);
				TIM_OC4PolarityConfig(TIM2,TIM_OCPolarity_High);//设置成上升沿捕获
				if(irSta&0x10)//完成一次高电平捕获
				{
					if(irSta&0x80)//是否已经捕获到引导码
					{
						if(timerValue>300&&timerValue<800)//标准值为560	即560us
						{
							keyValue<<=1;
							keyValue &=~1;
						}
						else if(timerValue>1400&&timerValue<1800)//标准值为1680  即1680us
						{
							keyValue<<=1;
							keyValue |= 1;
						}
						
					}
					else if(timerValue>4200&&timerValue<4700)//标准值为4500  即4.5ms
					{
						irSta |= (1<<7);//标记已经捕获到引导码
						keyCnt=0;//清除按键次数
					}
					else if(timerValue>2200&&timerValue<2600)
					{
						irSta &= ~((1<<4)&(1<<7));
						keyCnt++;
					}
					irSta &= ~(1<<4);//将上升沿标志位清0
				}		
			}
		}
		TIM_ClearFlag(TIM2,TIM_FLAG_CC4|TIM_IT_Update);//清中断请求标志位
}
/*获取按键值*/
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
/*处理按键的任务*/
void Task_key_deal(void *pData)
{
	uint8_t error=0;
	uint8_t value=0;
	while(1)
	{
		value=0;
		OSSemPend(keyPressed,0,&error);//等待信号量
		value=irDa_getKeyValue();
		/*以下是对按键的各种处理策略*/
		if((gameState&GAME_Pause_Mask)!=0&&(gameState&GAME_Over_Mask)!=0)//当前状态不为暂停状态且游戏不为结束状态
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
					//重新开始游戏
					break;
				default:
					break;
			}
		}
		//如果游戏结束,则up down left right pause 5个按键不可用
		else if((gameState&GAME_Over_Mask)==0)
		{
			if(value==194)//如果键值为reset,则重新开始游戏
			{
				
			}
			
		}
		else if((gameState&GAME_Pause_Mask)==0 && (gameState&GAME_Over_Mask)!=0)//如果当前游戏为暂停状态 
		{
				if(value==168)//如果键值为pause 
				{
					OSTmrStart(os_tmr1,(uint8_t*)&error);//重新启动软件定时器
					gameState|=GAME_Pause_Mask;//将游戏状态标记位非暂停状态
				}
				else if(value==194)//如果键值为reset,则重新开始游戏
				{
					
				}
		}
	}
}

