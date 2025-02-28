/**
 ****************************************************************************************************
 * @file        btim.c
 * @author      ALIENTEK
 * @version     V1.0
 * @date        2022-04-20
 * @brief       基本定时器TIM6驱动头文件
 * @license     MIT
 ****************************************************************************************************
 * @attention
 *
 *
 ****************************************************************************************************
 */

#ifndef __BTIM_H
#define __BTIM_H

#include "./SYSTEM/sys/sys.h"


/******************************************************************************************/
/* 基本定时器定义 */

/* TIMX 中断定义 
 * 默认为TIM6/TIM7
 */
 
#define BTIM_TIMX_INT                       TIM6    
#define BTIM_TIMX_INT_IRQn                  TIM6_DAC_IRQn
#define BTIM_TIMX_INT_IRQHandler            TIM6_DAC_IRQHandler
#define BTIM_TIMX_INT_CLK_ENABLE()          do{ __HAL_RCC_TIM6_CLK_ENABLE(); }while(0)  

/******************************************************************************************/

void btim_timx_int_init(uint16_t arr, uint16_t psc);                                    /* 基本定时器初始化函数 */

#endif

















