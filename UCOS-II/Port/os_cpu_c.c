/***************************************************************************************************************
**file_name :os_cpu_c.c
**Author    :Zhang
**Date:2016.4.3
**Description:ucosii在STM32上移植的C语言部分，包括任务堆栈初始化和钩子函数等
******************************************************************************************************************/
#define OS_CPU_GLOBALS
#include "ucos_ii.h"
/*********************************************************************************************************
*                                          LOCAL VARIABLES
*********************************************************************************************************/
#if OS_TMR_EN > 0
static  INT16U  OSTmrCtr;
#endif
/*****************************************************************************************************************
**                                      定义systick用到的几个寄存器以及几个常量
**
**SysTick控制及状态寄存器（地址：0xE000_E010）
**SysTick重装载数值寄存器（地址：0xE000_E014）
**SysTick当前数值寄存器（地址：0xE000_E018）
**SysTick校准数值寄存器（地址：0xE000_E01C）
**Systick中断优先级寄存器（地址：0xE000_ED23）
*****************************************************************************************************************/
#define  OS_CPU_CM3_NVIC_ST_CTRL    (*((volatile INT32U *)0xE000E010))   /* SysTick Ctrl & Status Reg. */
#define  OS_CPU_CM3_NVIC_ST_RELOAD  (*((volatile INT32U *)0xE000E014))   /* SysTick Reload  Value Reg. */
#define  OS_CPU_CM3_NVIC_ST_CURRENT (*((volatile INT32U *)0xE000E018))   /* SysTick Current Value Reg. */
#define  OS_CPU_CM3_NVIC_ST_CAL     (*((volatile INT32U *)0xE000E01C))   /* SysTick Cal     Value Reg. */
#define  OS_CPU_CM3_NVIC_PRIO_ST    (*((volatile INT8U  *)0xE000ED23))   /* SysTick Handler Prio  Reg. */

#define  OS_CPU_CM3_NVIC_ST_CTRL_COUNT                    0x00010000     /* Count flag.                */
#define  OS_CPU_CM3_NVIC_ST_CTRL_CLK_SRC                  0x00000004     /* Clock Source.              */
#define  OS_CPU_CM3_NVIC_ST_CTRL_INTEN                    0x00000002     /* Interrupt enable.          */
#define  OS_CPU_CM3_NVIC_ST_CTRL_ENABLE                   0x00000001     /* Counter mode.              */
#define  OS_CPU_CM3_NVIC_PRIO_MIN                               0xFF     /* Min handler prio.          */


/******************************************************************************************************************
**                      以下为用户自定义钩子函数，不做修改
**
******************************************************************************************************************/
#if OS_CPU_HOOKS_EN > 0 && OS_VERSION > 203
void  OSInitHookBegin (void)
{
#if OS_TMR_EN > 0
    OSTmrCtr = 0;
#endif
}
#endif

#if OS_CPU_HOOKS_EN > 0 && OS_VERSION > 203
void  OSInitHookEnd (void)
{
}
#endif

#if OS_CPU_HOOKS_EN > 0
void  OSTaskCreateHook (OS_TCB *ptcb)
{
#if OS_APP_HOOKS_EN > 0
    App_TaskCreateHook(ptcb);
#else
    (void)ptcb;                                  /* Prevent compiler warning                           */
#endif
}
#endif


#if OS_CPU_HOOKS_EN > 0
void  OSTaskDelHook (OS_TCB *ptcb)
{
#if OS_APP_HOOKS_EN > 0
    App_TaskDelHook(ptcb);
#else
    (void)ptcb;                                  /* Prevent compiler warning                           */
#endif
}
#endif


#if OS_CPU_HOOKS_EN > 0 && OS_VERSION >= 251
void  OSTaskIdleHook (void)
{
#if OS_APP_HOOKS_EN > 0
    App_TaskIdleHook();
#endif
}
#endif


#if OS_CPU_HOOKS_EN > 0
void  OSTaskStatHook (void)
{
#if OS_APP_HOOKS_EN > 0
    App_TaskStatHook();
#endif
}
#endif
void OSTaskReturnHook(OS_TCB * pcb)
{
	
}


#if (OS_CPU_HOOKS_EN > 0) && (OS_TASK_SW_HOOK_EN > 0)
void  OSTaskSwHook (void)
{
#if OS_APP_HOOKS_EN > 0
    App_TaskSwHook();
#endif
}
#endif


#if OS_CPU_HOOKS_EN > 0 && OS_VERSION > 203
void  OSTCBInitHook (OS_TCB *ptcb)
{
#if OS_APP_HOOKS_EN > 0
    App_TCBInitHook(ptcb);
#else
    (void)ptcb;                                  /* Prevent compiler warning                           */
#endif
}
#endif


#if (OS_CPU_HOOKS_EN > 0) && (OS_TIME_TICK_HOOK_EN > 0)
void  OSTimeTickHook (void)
{
#if OS_APP_HOOKS_EN > 0
    App_TimeTickHook();
#endif

#if OS_TMR_EN > 0
    OSTmrCtr++;
    if (OSTmrCtr >= (OS_TICKS_PER_SEC / OS_TMR_CFG_TICKS_PER_SEC)) {
        OSTmrCtr = 0;
        OSTmrSignal();
    }
#endif
}
#endif

/*********************************************************************************************************
*OSTaskStkInit(void (*task)(void *p_arg), void *p_arg, OS_STK *ptos, INT16U opt)
*Description:初始化任务堆栈的
*return:返回任务栈顶指针
*********************************************************************************************************/
OS_STK *OSTaskStkInit(void (*task)(void *p_arg),void *p_arg,OS_STK *ptos,INT16U opt)
{
    OS_STK *os_stk;                                                             //用来指向栈顶
    (void)opt;                                                                  //无用参数，防止编译器警告
    os_stk = ptos;                                                              //将任务栈底地址赋值给os_stk
    //寄存器站位
    *os_stk     = (INT32U)0x01000000L;                                         //首先压入xPSR状态寄存器 
    *(--os_stk) = (INT32U)task;                                                //任务函数入口地址
    *(--os_stk) = (INT32U)0x14141414L;                                         //压入LR(R14)寄存器，程序返回
    *(--os_stk) = (INT32U)0x12121212L;                                         //压入R12
    
    *(--os_stk) = (INT32U)0x03030303L;                                         //R3
    *(--os_stk) = (INT32U)0x02020202L;                                         //R2
    *(--os_stk) = (INT32U)0X01010101L;                                         //R1
    *(--os_stk) = (INT32U)p_arg;                                               //R0
    
                                                    
    *(--os_stk)  = (INT32U)0x11111111L;                                         // R11                                       
    *(--os_stk)  = (INT32U)0x10101010L;                                         // R10                         
    *(--os_stk)  = (INT32U)0x09090909L;                                         // R9                          
    *(--os_stk)  = (INT32U)0x08080808L;                                         //R8                          
    *(--os_stk)  = (INT32U)0x07070707L;                                         // R7                           
    *(--os_stk)  = (INT32U)0x06060606L;                                         // R6                          
    *(--os_stk)  = (INT32U)0x05050505L;                                         // R5                          
    *(--os_stk)  = (INT32U)0x04040404L;                                         // R4
    return os_stk;
          
}//end OSTaskStkInit

/*********************************************************************************************************
*                                         SysTick_Handler()
*
* Description: Handle the system tick (SysTick) interrupt, which is used to generate the uC/OS-II tick
*              interrupt.
*
* Arguments  : none.
*
* Note(s)    : 1) This function MUST be placed on entry 15 of the Cortex-M3 vector table.
*********************************************************************************************************/

void  SysTick_Handler (void)
{
    OS_CPU_SR  cpu_sr;


    OS_ENTER_CRITICAL();                         /* Tell uC/OS-II that we are starting an ISR          */
    OSIntNesting++;
    OS_EXIT_CRITICAL();

    OSTimeTick();                                /* Call uC/OS-II's OSTimeTick()                       */

    OSIntExit();                                 /* Tell uC/OS-II that we are leaving the ISR          */
}

/*
*********************************************************************************************************
*                                          OS_CPU_SysTickInit()
*
* Description: Initialize the SysTick.
*
* Arguments  : none.
*
* Note(s)    : 1) This function MUST be called after OSStart() & after processor initialization.
*********************************************************************************************************
*/

void  OS_CPU_SysTickInit (void)
{
    INT32U  cnts;


    cnts = OS_CPU_SysTickClkFreq() / OS_TICKS_PER_SEC;

    OS_CPU_CM3_NVIC_ST_RELOAD = (cnts - 1);
                                                 /* Set prio of SysTick handler to min prio.           */
    OS_CPU_CM3_NVIC_PRIO_ST   = OS_CPU_CM3_NVIC_PRIO_MIN;
                                                 /* Enable timer.                                      */
    OS_CPU_CM3_NVIC_ST_CTRL  |= OS_CPU_CM3_NVIC_ST_CTRL_CLK_SRC | OS_CPU_CM3_NVIC_ST_CTRL_ENABLE;
                                                 /* Enable timer interrupt.                            */
    OS_CPU_CM3_NVIC_ST_CTRL  |= OS_CPU_CM3_NVIC_ST_CTRL_INTEN;
}


















