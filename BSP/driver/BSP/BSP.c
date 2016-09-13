#include "BSP.h"
#include "stm32f10x.h"
INT32U OS_CPU_SysTickClkFreq(void)
{
  RCC_ClocksTypeDef rcc_sysClocks;
  RCC_GetClocksFreq(&rcc_sysClocks);
  return (INT32U)(rcc_sysClocks.HCLK_Frequency);
}
