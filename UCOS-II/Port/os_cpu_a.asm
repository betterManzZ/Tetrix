***************************************************************************************************************
*file_name :os_cpu_a.asm
*Author    :Zhang
;*Date:2016.4.3
;*Description:ucosii在STM32上移植的汇编语言部分，主要与任务调度相关的底层切换函数
;*****************************************************************************************************************
    IMPORT  OSRunning                                           ; External references
    IMPORT  OSPrioCur
    IMPORT  OSPrioHighRdy
    IMPORT  OSTCBCur
    IMPORT  OSTCBHighRdy
    IMPORT  OSIntNesting
    IMPORT  OSIntExit
    IMPORT  OSTaskSwHook


    EXPORT  OS_CPU_SR_Save                                      ; Functions declared in this file
    EXPORT  OS_CPU_SR_Restore
    EXPORT  OSStartHighRdy
    EXPORT  OSCtxSw
    EXPORT  OSIntCtxSw
    EXPORT  PendSV_Handler
;*****************************************************************************************************************
; 常量定义      
;*****************************************************************************************************************/
NVIC_INT_CTRL   EQU     0xE000ED04                              ;中断及状态控制寄存器ICSR地址
NVIC_SYSPRI14   EQU     0xE000ED22                              ; pendSV中断优先级配置寄存器地址!!!!!!!!
NVIC_PENDSV_PRI EQU     0xFF                                    ; 将pendSV中断最低优先级!!!!!!!!
NVIC_PENDSVSET  EQU     0x10000000                              ;pendSV中断悬起位  写1有效

    AREA |.text|, CODE, READONLY, ALIGN=2
    THUMB
    REQUIRE8
    PRESERVE8
;*****************************************************************************************************************
;进入、退出临界段函数
;OS_CPU_SR OS_CPU_SR_Save(void)
;void OS_CPU_SR_Restore(OS_CPU_SR)
;*****************************************************************************************************************
OS_CPU_SR_Save
  MRS   R0,PRIMASK                                              ;保存PRIMASK(中断)寄存器的值到R0中,作为函数的返回值
  CPSID I                                                       ;关中断汇编指令
  BX    LR                                                     ;返回调用程序

OS_CPU_SR_Restore
  MSR   PRIMASK,R0                                              ;
  BX    LR
;*****************************************************************************************************************
;void OSStartHighRdy(void)
; Note(s) : 1) This function triggers a PendSV exception (essentially, causes a context switch) to cause
;              the first task to start.
;
;           2) OSStartHighRdy() MUST:
;              a) Setup PendSV exception priority to lowest;
;              b) Set initial PSP to 0, to tell context switcher this is first run;
;              c) Set OSRunning to TRUE;
;              d) Trigger PendSV exception;
;              e) Enable interrupts (tasks will run with interrupts enabled).
;*****************************************************************************************************************/
OSStartHighRdy
   ;Setup PendSV exception priority to lowest;
   LDR  R0,= NVIC_SYSPRI14                                        ;将pendSV优先级寄存器地址送到R0
   LDR  R1,= NVIC_PENDSV_PRI                                      ;将最低优先级送到R1
   STRB R1,[R0]                                                   
   
   ;Set initial PSP to 0, to tell context switcher this is first run;
   MOVS R0,#0                                                    ;
   MSR  PSP,R0 
   
   ;Set OSRunning to TRUE
   LDR R1,=OSRunning                                             ;
   MOVS R2,#1                                                     ;
   STRB R2,[R1]                                                   ;
   
   ;Trigger PendSV exception
   
   LDR  R0,=NVIC_INT_CTRL                                   
   LDR  R1,=NVIC_PENDSVSET     
   STR  R1,[R0]
   
   ;Enable interrupts
    CPSIE I                                                      ;enable interrupts
    
;Should never get here
OSStartHang
   B    OSStartHang

;*****************************************************************************************************************
;OSCtxSw()
;OSIntCtxSw()
;*****************************************************************************************************************/
OSCtxSw
   LDR  R0,=NVIC_INT_CTRL                                       
   LDR  R1,=NVIC_PENDSVSET
   STR  R1,[R0]
   BX   LR
OSIntCtxSw
   LDR  R0,=NVIC_INT_CTRL                                       
   LDR  R1,=NVIC_PENDSVSET
   STR  R1,[R0]
   BX   LR

;*****************************************************************************************************************
;void PendSV_Handler(void); 
;              a) Get the process SP, if 0 then skip (goto d) the saving part (first context switch);
;              b) Save remaining regs r4-r11 on process stack;
;              c) Save the process SP in its TCB, OSTCBCur->OSTCBStkPtr = SP;
;              d) Call OSTaskSwHook();
;              e) Get current high priority, OSPrioCur = OSPrioHighRdy;
;              f) Get current ready thread TCB, OSTCBCur = OSTCBHighRdy;
;              g) Get new process SP from TCB, SP = OSTCBHighRdy->OSTCBStkPtr;
;              h) Restore R4-R11 from new process stack;
;              i) Perform exception return which will restore remaining context.
;*****************************************************************************************************************/
PendSV_Handler
  CPSID I                                                          ;关中断
  MRS R0,PSP                                                       ;
  CBZ R0,PendSV_Handler_nosave                               ;如果psp寄存器为0，代表系统第一次启动任务，不需要保存系统现场
  ;psp不为0,需要保存r4-r11 8个寄存器的值存到psp中
  SUBS R0,#0X20                                                    ;
  STM R0,{R4-R11}                                                 ;这里采用的是 别名(alias)寄存器访问MPU ，具体参考Cotex-M3权威指南chpt14
  
  LDR R1,=OSTCBCur                                                 ;这里讲OSTCBCur指针变量的地址赋值给R0
  LDR R1,[R1]                                                      ;这里R0指向当前TCB
  STR R0,[R1]                                                      ;将栈顶指针入栈
PendSV_Handler_nosave
  ;d) Call OSTaskSwHook();
  PUSH {R14}                                                       ;保存LR寄存器的值到MSP
  LDR R0,=OSTaskSwHook                                             ;
  BLX R0                                                           ;
  POP {R14}                                                        ;
  ;e) Get current high priority, OSPrioCur = OSPrioHighRdy;
  LDR R0,=OSPrioCur
  LDR R1,=OSPrioHighRdy                                         
  LDRB R1,[R1]                                                     ;OSPrioCur，OSPrioHighRdy是uint8数据类型
  STRB R1,[R0]
  ;f) Get current ready thread TCB, OSTCBCur = OSTCBHighRdy
  LDR R0,=OSTCBCur                                                  ;
  LDR R1,=OSTCBHighRdy                                              ;
  LDR R1,[R1]                                                       ;
  STR R1,[R0]                                                       ;R1指向OSTCBHighRdy->OSTCBStkPtr
  ;g) Get new process SP from TCB, SP = OSTCBHighRdy->OSTCBStkPtr
  LDR R0,[R1]                                                       ;R0存放程序堆栈OSTCBStkPtr里的地址
  LDM R0,{R4-R11}                                                   ;将程序堆栈中后入栈的R4-R11弹出
  ADDS R0,#0X20                                                     ;
  
  MSR PSP,R0
  ORR LR,LR,#0x04                                                   ;将LR第2位置1，返回到用户模式下
  CPSIE I                                                           ;开中断
  BX      LR                                                        ;返回
  END


