#ifndef __tetrix__h_
#define __tetrix__h_
#include "stdint.h"
#include "ucos_ii.h"
/**************************************************************************************************
																				�궨��
***************************************************************************************************/
typedef uint8_t BOOLEAN;
#define	TRUE	1
#define FALSE 0
#define GAME_Over_Mask	0x01
#define GAME_Pause_Mask 0X02
#define TETRIX_WIDTH (160)   //������Ϸ����Ŀ��
#define TETRIX_HEIGHT  (320)   //������Ϸ����ĸ߶�
#define BLOCK_LENGTH (16)     //ÿ��С��ĳ���
#define GRIDX_NUM    (TETRIX_WIDTH/BLOCK_LENGTH)  //����X������С�������   �����4��4����������Χǽ���޶�С�����ƶ���λ��
#define GRIDY_NUM		 (TETRIX_HEIGHT/BLOCK_LENGTH)  //����Y������С�������

#define NextShape_GridX				(11)
#define NextShape_GridY				(3)
#define CurrentShape_GridX		(4)
#define CurrentShape_GridY		(-2)
//�������
#define Task_tetrix_mainFrame_STK_SIZE 512
#define Task_tetrix_mainFrame_Prio	30
extern OS_STK	Task_tetrix_mainFrame_STK[Task_tetrix_mainFrame_STK_SIZE];
extern OS_TMR *os_tmr1;
//��Ϸ���
#define FALLING_SPEED					(100)			//1000MS
//����7�л�����״���
#define shapeLine   1
#define shapeL      2
#define shapeopL    3  
#define shapeSquare 4 
#define shapeZ	    5 
#define shapeopZ    6   
#define shapeT      7  //��

/**************************************************************************************************
																			���ݽṹ����
***************************************************************************************************/
extern uint16_t const shape4X4map[7][4];

typedef struct baseShape{
	uint8_t shape_id;           //7�л�����״�е�һ��  0-6
	uint8_t state;						   //ÿ����״��Ӧ������������״̬ 0-3
	 
	INT8S	gridX;		
	INT8S gridY;
	uint16_t color;
}baseShape;



/**************************************************************************************************
																		ȫ�ֱ�������
***************************************************************************************************/
extern uint16_t Tetrix_grid_map[GRIDY_NUM];
extern baseShape currentShape;
extern baseShape nextShape;	
extern uint16_t current_score;//score
extern uint8_t gameState;
/**************************************************************************************************
																			��������
***************************************************************************************************/
void Task_tetrix_startGame(void *pData);
void _cb_refreshMainFrame(void *ptmr, void *parg);
void Tetrix_init(void);
void TetrixXYConvertLcdXY(uint8_t gridX,uint8_t gridY,uint16_t *x,uint16_t *y);
void Tetrix_drawBlock(uint8_t gridX,uint8_t gridY,uint16_t color);
void Tetrix_drawShape(baseShape *shape);
void Tetrix_create_NewShapeInRandom(baseShape *shape);

BOOLEAN Tetrix_isToTheBot(const baseShape *shape);//�������������ж��иı�Shape��״̬
BOOLEAN Tetrix_isToTheLeft(const baseShape *shape);
BOOLEAN Tetrix_isToTheRight(const baseShape *shape);
void Tetrix_clearFullLine(void);

void Tetrix_setGridMapTrue(uint8_t gridX,uint8_t gridY);
void Tetrix_unsetGridMapTrue(uint8_t gridX,uint8_t gridY);
void Tetrix_unsetGridMapTrue(uint8_t gridX,uint8_t gridY);

void Tetrix_ChangeState(void);
void Tetrix_moveLeft(void);
void Tetrix_moveRight(void);
void Tetrix_moveDown(void);
#endif

