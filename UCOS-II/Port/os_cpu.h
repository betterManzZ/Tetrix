/***************************************************************************************************************
**file_name :os_cpu.h
**Author    :Zhang
**Date:2016.4.3
**Description:ucosii在STM32上移植的C语言部分，包括任务堆栈初始化和钩子函数等
******************************************************************************************************************/
#ifndef __OS_CPU_H__
#define __OS_CPU_H__

#ifdef OS_CPU_GLOBALS
#define OS_CPU_EXT
#else
#define OS_CPU_EXT extern
#endif
/*****************************************************************************************************************
** 定义与编译器无关的数据类型                             
*****************************************************************************************************************/
typedef unsigned char BOOLEAN;
typedef unsigned char INT8U;
typedef signed char INT8S;
typedef unsigned short INT16U;
typedef signed short INT16S;
typedef unsigned int INT32U;
typedef signed int INT32S;
typedef float FP32;
typedef double FP64;
//stm32是32bit数据宽度，所以OS_STK和OS_CPU_SR都是32bit
typedef unsigned int OS_STK;//ucosii任务堆栈数据类型
typedef unsigned int OS_CPU_SR;//cpu状态寄存器

//定义堆栈递增方向
#define OS_STK_GROWTH           1                                 //定义堆栈增长方向递增
#define OS_TASK_SW()            OSCtxSw()                         //定义任务切换宏



/*****************************************************************************************************************
**                                       定义进入临界段的方法
** OS_CRITICAL_METHOD           1                                 // 直接运用中断开关指令实现
** OS_CRITICAL_METHOD           2                                 //利用堆栈保存和恢复CPU状态
** OS_CRITICAL_METHOD           3                                 //将CPU状态保存到cpi_sr中，退出临界段后直接恢复
*****************************************************************************************************************/

#define OS_CRITICAL_METHOD	3                                 //进入临界段的方法3

 
//如果定义了方法三，就申明一下方法，在os_cpu_a.asm中实现
#if OS_CRITICAL_METHOD   ==     3u                                
OS_CPU_EXT OS_CPU_SR OS_CPU_SR_Save(void);                                    //申明函数 OS_CPU_SR_Save()
void       OS_CPU_SR_Restore(OS_CPU_SR cpu_sr);                            //申明函数 OS_CPU_SR_Restore(OS_CPU_SR)
#endif

#if OS_CRITICAL_METHOD == 3u
#define OS_ENTER_CRITICAL()     {cpu_sr = OS_CPU_SR_Save();}    //定义进入临界段宏
#define OS_EXIT_CRITICAL()      {OS_CPU_SR_Restore(cpu_sr);}    //定义退出临界段宏
#endif

  
/*****************************************************************************************************************
                                            函数申明
说明：以下申明的函数均在os_cpu_a.asm中用汇编实现 
*****************************************************************************************************************/
void OSStartHighRdy(void);                                        //开始启动最高优先级任务
void OSCtxSw(void);                                               //triggers the PendSV exception，悬起PendSV异常
void OSIntCtxSw(void);                                            //与OSCtxSw几乎相同
void PendSV_Handler(void);                                  //上下文切换
//OS_CPU_SR OS_CPU_SR_Save(void);
//void OS_CPU_SR_Restore(OS_CPU_SR);
void OS_CPU_SysTickInit(void);
#endif












