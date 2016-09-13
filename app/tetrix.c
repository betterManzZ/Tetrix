#include "tetrix.h"
#include "lcd.h"
#include "stdlib.h"
#include "time.h"
#include "ucos_ii.h"
#include "adc.h"
#include "irDa.h"
/*
将整个240X320的液晶屏分为两部分，左边(160X320)显示游戏过程，右边(80X320)显示下一个形状以及score等信息
将左边(160X320)划分成网格，每小格16X16像素，一共为10X20

		_0_1_2_3_4_5_6_7_8_9________________________
  0	| | |	| | |	| | |	| |
		------------------------------------------
	1 | | |	| | | | | |	| |
		------------------------------------------
	2 | | |	| | | | | |	| |
		------------------------------------------
	3 | | |	| | | | | |	| |
		------------------------------------------
	4 | | |	| | | | | |	| |
		------------------------------------------
	5 | | | | | | | | |	| |
		------------------------------------------
	6 | | | | | | | | |	| |
		------------------------------------------
	7 | | | | | | | | |	| |
	.
	.

*/
//格点状态表
//标示每个小格子的状态0|1，X轴上由低10位标示，高6位不用
uint16_t Tetrix_grid_map[GRIDY_NUM];

//下列7个数组表示基本图形在16宫格中的编码 shape表示在16宫格
//左->右取模   低位->高位
uint16_t const shape4X4map[7][4] = {{0x00f0,0x4444,0x0f00,0x2222},/*--*/\
																		{0X0071,0X0226,0X0470,0X0322},/*L*/\
																		{0X0074,0X0622,0X0170,0X0223},/*_|*/\
																		{0X0066,0x0066,0x0066,0x0066},/*口*/\
																		{0X0036,0X0462,0X0360,0X0231},/*_|-*/\
																		{0X0072,0X0262,0X0270,0X0232},/*土*/\
																		{0x0063,0x0264,0x0630,0x0132}}/*Z*/;

baseShape currentShape;//the current shape
baseShape nextShape;//the next shape
uint16_t current_score=0;
/*
		bit0:0 game over | 1 game is running
		bit1:0 game is pause | 1 game is not pause
		其它位保留
*/																		
uint8_t gameState=0xff;//标示游戏当前状态												
OS_TMR *os_tmr1;
OS_STK	Task_tetrix_mainFrame_STK[Task_tetrix_mainFrame_STK_SIZE];																	
/*
功能：初始化游戏
			LCD初始化 
			红外初始化
			设置背景颜色
*/

void Tetrix_init()
{
	uint8_t i =0;
	LCD_Init();
	//设置背景颜色
	LCD_Clear(0,0,240,320,BACKGROUND);
	//初始化格点状态表
	for(i=0;i<GRIDY_NUM;i++)
	{
		Tetrix_grid_map[i] = 0;
	}
}

/*
	功能：将游戏中grid中的坐标转化为lcd中的坐标
*/
void TetrixXYConvertLcdXY(uint8_t gridX,uint8_t gridY,uint16_t *x,uint16_t *y)
{
	if(gridX < 0 ||gridX >= GRIDX_NUM+5 || gridY < 0 || gridY >= GRIDY_NUM)//输入错误
	{
		return ;
	}
	*x = BLOCK_LENGTH * gridX;
	*y = BLOCK_LENGTH * gridY;
}
/*
	功能：在LCD上画出一个小方块
	参数：girdX表示小格的横坐标(0-9)
				girdY表示小格的纵坐标(0-19)
*/
void Tetrix_drawBlock(uint8_t gridX,uint8_t gridY,uint16_t color)
{
	uint16_t x,y;
	TetrixXYConvertLcdXY(gridX,gridY,&x,&y);
	LCD_Clear(x,y,BLOCK_LENGTH,BLOCK_LENGTH,color);
}
//画出一个图形
void Tetrix_drawShape(baseShape *shape)
{
	int i=0;
	uint16_t temp;
	uint16_t flag = 0x0001;
	INT8S gridX = shape->gridX;
	INT8S gridY = shape->gridY;
	if(shape==(void*)0)
	{
		return ;
	}
	shape->state = (shape->state)%4;
	temp = shape4X4map[shape->shape_id][shape->state];
	for(i=1;i<=16;i++)
	{
		//gridX<0 gridY<0的部分不显示
		if(gridY>=0&&gridX>=0)
		{
			if((temp&flag)!=0)
			{
				Tetrix_drawBlock(gridX,gridY,shape->color);
			}
		}
		gridX++;
		if(i%4==0)
		{
			gridY++;
			gridX = shape->gridX;
		}	
		flag = flag<<1;
	}
	
}
/*
Description:This function is called to clear the shape which has been display, without  effect on other bolck
Arguments:shape					is a pointer to the data type--baseShape
Returns:void
*/
void Tetrix_clearShape(baseShape *shape)
{
	int i=0;
	uint16_t temp;
	uint16_t flag = 0x0001;
	uint8_t gridX = shape->gridX;
	uint8_t gridY = shape->gridY;
	if(shape==(void*)0)
	{
		return ;
	}
	shape->state = (shape->state)%4;
	temp = shape4X4map[shape->shape_id][shape->state];
	for(i=1;i<=16;i++)
	{
		if((temp&flag)!=0)
		{
			Tetrix_drawBlock(gridX,gridY,BACKGROUND);
		}
		gridX++;
		if(i%4==0)
		{
			gridY++;
			gridX = shape->gridX;
		}	
		flag = flag<<1;
	}
	
}
/*读取小方块的颜色*/
uint16_t Tetrix_getBlockColor(INT8S gridX,INT8S gridY)
{
	uint16_t x,y;
	TetrixXYConvertLcdXY(gridX,gridY,&x,&y);
	return (LCD_GetPoint(x,y));
}
/*
移动一行小方块到另一行
*/
void Tetrix_moveLineAToB(uint8_t A,uint8_t B)
{
	uint16_t i;
	uint16_t color;
	uint16_t temp = Tetrix_grid_map[A];
	if(A==B)
	{
		return;
	}
	for(i=0;i<10;i++)
	{
		if((temp&(1<<i))!=0)
		{
			//读取(A,i)坐标处的小方块颜色值，并写入坐标(B,i)
			color = Tetrix_getBlockColor(i,A);
			Tetrix_drawBlock(i,B,color);
		}
		else
		{
			Tetrix_drawBlock(i,B,BACKGROUND);
		}
		Tetrix_drawBlock(i,A,BACKGROUND);//将(A,i)坐标处变为背景色
	}
	//改变标志位
	Tetrix_grid_map[B] = Tetrix_grid_map[A];
	Tetrix_grid_map[A] = 0;
}
/*直接清除一行*/
void Tetrix_clearLineDri(uint8_t row)
{
	LCD_Clear(0,row,BLOCK_LENGTH*GRIDX_NUM,BLOCK_LENGTH,BACKGROUND);
}
/*
功能：产生一个随机的图形
*/
void Tetrix_create_NewShapeInRandom(baseShape *shape)
{
	
		
		srand(Get_Adc(1));  /*get the ADC value for generate a seed*/
		shape->shape_id = rand()%7;//产生0-6之间的随机数
		shape->state = rand()%4;//产生0-3之间的随机数
		switch (shape->shape_id)
		{
			case 0:
				shape->color = BLUE;
				break;
			case 1:
				shape->color = BLUE2;
				break;
			case 2:
				shape->color = RED;
				break;
			case 3:
				shape->color = MAGENTA;
				break;
			case 4:
				shape->color = GREEN;
				break;
			case 5:
				shape->color = CYAN;
				break;
			case 6:
				shape->color = YELLOW;
				break;
		}
		shape->gridX = CurrentShape_GridX;
		shape->gridY = CurrentShape_GridY;
}
/*
Description:设置格点标记位为1
Arguments:(gridX,gridY)小方框坐标
*/
void Tetrix_setGridMapTrue(uint8_t gridX,uint8_t gridY)
{
	if(gridX>=GRIDX_NUM || gridY>=GRIDY_NUM)//参数不合法
		return ;
	Tetrix_grid_map[gridY] |= (1<<gridX);
}
void Tetrix_setGridMapByShape(baseShape* shape)
{
	int i=0;
	uint16_t temp;
	uint16_t flag = 0x0001;
	uint8_t gridX = shape->gridX;
	uint8_t gridY = shape->gridY;
	if(shape==(void*)0)
	{
		return ;
	}
	shape->state = (shape->state)%4;
	temp = shape4X4map[shape->shape_id][shape->state];
	for(i=1;i<=16;i++)
	{
		if((temp&flag)!=0)
		{
			Tetrix_setGridMapTrue(gridX,gridY);
		}
		gridX++;
		if(i%4==0)
		{
			gridY++;
			gridX = shape->gridX;
		}	
		flag = flag<<1;
	}
}
/*
Description:设置格点标记位为0
Arguments:(gridX,gridY)小方框坐标
*/
void Tetrix_unsetGridMapTrue(uint8_t gridX,uint8_t gridY)
{
	if(gridX<0||gridX>=GRIDX_NUM || gridY<0 || gridY>=GRIDY_NUM)//参数不合法
		return ;
	Tetrix_grid_map[gridX] &= ~(1<<gridY);
}
/*
Description:判断是否是否到达底部
Return:如果到达底部返回TRUE 否则返回FALSE
*/
BOOLEAN Tetrix_isToTheBot(const baseShape *shape)//不允许在条件判断中改变Shape的状态
{
	int i=0;
	uint16_t temp;
	uint16_t flag = 0x0001;
	INT8S gridX = shape->gridX;
	INT8S gridY = shape->gridY;//现将图形下移1个，如果出现覆盖，则说明当前图形已到底部
	temp = shape4X4map[shape->shape_id][shape->state];
	for(i=1;i<=16;i++)
	{
		if(gridX>=0&&gridY>=0)
		{
			if((temp&flag)!=0)
			{
				if( (Tetrix_grid_map[gridY]&(1<<gridX) )!=0 || (gridY > GRIDY_NUM-1))//只要有一个点在Tetrix_grid_map中已被标记为1，则返回TRUE
				{
					return TRUE;
				}
			}
		}
		gridX++;
		if(i%4==0)
		{
			gridY++;
			gridX = shape->gridX;
		}	
		flag = flag<<1;
	}
	
	return FALSE;
}
/*
Description:判断是否到达最左端
*/
BOOLEAN Tetrix_isToTheLeft(const baseShape *shape)
{
	int i=0;
	uint16_t temp;
	uint16_t flag = 0x0001;
	INT8S gridX = shape->gridX;
	INT8S gridY = shape->gridY;
	temp = shape4X4map[shape->shape_id][shape->state];
	for(i=1;i<=16;i++)
	{
		if((temp&flag)!=0)
		{
			if( (gridX <0)||(Tetrix_grid_map[gridY]&(1<<gridX) )!=0)//只要有一个点在Tetrix_grid_map中已被标记为1，则返回TRUE
			{
				return TRUE;
			}
		}
		gridX++;
		if(i%4==0)
		{
			gridY++;
			gridX = shape->gridX;
		}	
		flag = flag<<1;
	}
	
	return FALSE;
}
/*判断是否到达最右端*/
BOOLEAN Tetrix_isToTheRight(const baseShape *shape)
{
	int i=0;
	uint16_t temp;
	uint16_t flag = 0x0001;
	INT8S gridX = shape->gridX;
	INT8S gridY = shape->gridY;
	temp = shape4X4map[shape->shape_id][shape->state];
	for(i=1;i<=16;i++)
	{
		if((temp&flag)!=0)
		{
			if( (gridX >=GRIDX_NUM)||(Tetrix_grid_map[gridY]&(1<<gridX) )!=0)//只要有一个点在Tetrix_grid_map中已被标记为1，则返回TRUE
			{
				return TRUE;
			}
		}
		gridX++;
		if(i%4==0)
		{
			gridY++;
			gridX = shape->gridX;
		}	
		flag = flag<<1;
	}
	
	return FALSE;
}


/*判断当前形状是否可变*/
/*
可变 则返回TRUE
*/
BOOLEAN Tetrix_shapeChangeable(baseShape *shape)
{
	BOOLEAN FLAG;
	uint8_t shapeStateBak = shape->state;//先保存当前状态
	//先试探改变当前形状的状态，判断是否会产生冲突
	shape->state+=1;
	shape->state%=4;
	if(Tetrix_isToTheBot(shape)==TRUE||Tetrix_isToTheLeft(shape)==TRUE || Tetrix_isToTheRight(&currentShape)==TRUE)//产生冲突
	{	
		FLAG = FALSE;
	}
	else
	{
		FLAG = TRUE;
	}
	shape->state = shapeStateBak;//恢复原来的状态
	return FLAG;
}
//查看当前是否有行满，有则清除该行，并下移上方形状
void Tetrix_clearFullLine()
{
	uint8_t row=0;
	uint8_t i=0;
	uint8_t ptr1=GRIDY_NUM-1;
	uint8_t ptr2=GRIDY_NUM-1;
	uint8_t flag[20];
	uint8_t isFull=0;
	for(i=0;i<20;i++)
	{
		flag[i] = 0;
	}
	for(row=GRIDY_NUM;row>0;row--)
	{
		if((Tetrix_grid_map[row-1]&0x3ff)==0x3ff)//该行已满
		{
			flag[row-1] = 1;
			isFull=1;
		}
		else if((Tetrix_grid_map[row-1]&0x3ff)==0)//上部分没有小方块，结束查找
		{
			flag[row-1] = 2;
			break;
		}
	}
	i=0;
	if(isFull==1)
	{
	while(flag[ptr1]!=2)
	{
		if(flag[ptr1]==0)
		{
				Tetrix_moveLineAToB(ptr1--,ptr2--);
		}
		else
		{
			i++;
			current_score+=10*i;//每消除一行得分
			LCD_Clear(185,220,20,40,BACKGROUND);
			LCD_DisNum(185,220,current_score,RED);
			ptr1--;
		}
	}	
	}
}








/*
Drescriptions:初始化游戏
*/
void Task_tetrix_startGame(void *pData)
{
		int8_t error;
		//set Dividing Line
		LCD_Clear(161,0,1,320,RED);
		current_score=0;
		//create a new shape
		Tetrix_create_NewShapeInRandom(&currentShape);
		Tetrix_create_NewShapeInRandom(&nextShape);
		
		nextShape.gridX = NextShape_GridX;
		nextShape.gridY = NextShape_GridY;
		//draw the first shape
		Tetrix_drawShape(&currentShape);
		/*游戏右半部分界面*/
		//the next shape
		Tetrix_drawShape(&nextShape);
		//set the text "next shape"
		LCD_DisStr(170,30,(uint8_t *)"next shape",RED);
		LCD_DisStr(185,200,(uint8_t *)"score",BLUE);
		LCD_DisNum(185,220,current_score,RED);
		os_tmr1=OSTmrCreate(FALLING_SPEED/(1000.0/OS_TMR_CFG_TICKS_PER_SEC),FALLING_SPEED/(1000.0/OS_TMR_CFG_TICKS_PER_SEC),OS_TMR_OPT_PERIODIC,(OS_TMR_CALLBACK)_cb_refreshMainFrame,0,(uint8_t *)"refreshFrame",(uint8_t*)&error);
		OSTmrStart(os_tmr1,(uint8_t*)&error);
		OSTaskDel(OS_PRIO_SELF);
		while(1)
		{;}
}
void _cb_refreshMainFrame(void *ptmr, void *parg)
{
	INT8U error=0;
#if OS_CRITICAL_METHOD ==3
	OS_CPU_SR cpu_sr;
#endif
	currentShape.gridY++;
	if(Tetrix_isToTheBot(&currentShape)==FALSE)//没有到达底部，将当前图形下移一格
	{
		currentShape.gridY--;
		Tetrix_clearShape(&currentShape);
		currentShape.gridY++;
		Tetrix_drawShape(&currentShape);
	}
	else
	{
		currentShape.gridY--;
		//设置标志位
		Tetrix_setGridMapByShape(&currentShape);
#if OS_CRITICAL_METHOD == 3
		OS_ENTER_CRITICAL();
#endif
		Tetrix_clearFullLine();
#if OS_CRITICAL_METHOD == 3
		OS_EXIT_CRITICAL();
#endif
		if(currentShape.gridY<=0)//游戏结束
		{
			OSTmrStop(os_tmr1,OS_TMR_OPT_NONE,(void*)0,(INT8U*)&error);
			//gameState&=~GAME_Over_Mask;//将游戏结束标志位置0
			LCD_DisStr(40,100,(uint8_t*)"game over!",RED);
			LCD_DisStr(40,120,(uint8_t*)"your score is",RED);
			LCD_DisNum(55,140,current_score,RED);
			return;
		}

		//开始下个图形
		currentShape = nextShape;
		currentShape.gridX = CurrentShape_GridX;
		currentShape.gridY = CurrentShape_GridY;	
		
		//chage the next shape
		Tetrix_clearShape(&nextShape);
		Tetrix_create_NewShapeInRandom(&nextShape);
		nextShape.gridX = NextShape_GridX;
		nextShape.gridY = NextShape_GridY;
		Tetrix_drawShape(&nextShape);		
		
		Tetrix_drawShape(&currentShape);
	}
}
//called when the key "up" is pressed
void Tetrix_ChangeState(void)
{
	//判断是否会产生冲突
	if(Tetrix_shapeChangeable(&currentShape)==TRUE)
	{
		Tetrix_clearShape(&currentShape);
		currentShape.state+=1;
		currentShape.state%=4;
		Tetrix_drawShape(&currentShape);
	}
}
void Tetrix_moveLeft(void)
{
	currentShape.gridX--;
	if(Tetrix_isToTheLeft(&currentShape)==FALSE)//判断是否可以移动
	{
		currentShape.gridX++;
		Tetrix_clearShape(&currentShape);
		currentShape.gridX--;
		Tetrix_drawShape(&currentShape);
	}
	else
	{
		currentShape.gridX++;
	}
}
void Tetrix_moveRight(void)
{
	currentShape.gridX++;
	if(Tetrix_isToTheRight(&currentShape)==FALSE)//判断是否可以移动
	{
		currentShape.gridX--;
		Tetrix_clearShape(&currentShape);
		currentShape.gridX++;
		Tetrix_drawShape(&currentShape);
	}
	else
	{
		currentShape.gridX--;
	}
}
void Tetrix_moveDown(void)
{
	currentShape.gridY++;
	if(Tetrix_isToTheBot(&currentShape)==FALSE)
	{
		currentShape.gridY--;
		Tetrix_clearShape(&currentShape);
		currentShape.gridY++;
		Tetrix_drawShape(&currentShape);
	}
	else
	{
		currentShape.gridY--;
	}
}
 


