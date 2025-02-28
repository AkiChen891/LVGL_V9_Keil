/**
 ****************************************************************************************************
 * @file        btim.c
 * @author      ALIENTEK
 * @version     V1.0
 * @date        2022-04-20
 * @brief       基本定时器TIM6驱动
 * @license     MIT
 ****************************************************************************************************
 * @attention
 *
 *
 ****************************************************************************************************
 */

#include "./BSP/LED/led.h"
#include "./BSP/TIMER/btim.h"
#include "lvgl.h"

TIM_HandleTypeDef g_timx_handle;                                /* 定时器句柄 */

/**
 * @brief       定时器TIMX初始化
 * @note
 *              定时器溢出时间 Tout = ((arr + 1) * (psc + 1)) / Ft us.
 *              Ft=定时器工作频率
 *              TIM6挂载在APB1定时器总线，频率90Mhz
 *
 * @param       arr : 自动重装计数器
 * @param       psc : 时钟预分频数
 * @retval      无
 */
void btim_timx_int_init(uint16_t arr, uint16_t psc)
{
    g_timx_handle.Instance = BTIM_TIMX_INT;                      /* 定时器x */
    g_timx_handle.Init.Prescaler = psc;                          /* 分频系数 */
    g_timx_handle.Init.CounterMode = TIM_COUNTERMODE_UP;         /* 递增计数 */
    g_timx_handle.Init.Period = arr;                             /* 自动重装载值 */
    HAL_TIM_Base_Init(&g_timx_handle);
    
    HAL_TIM_Base_Start_IT(&g_timx_handle);                       /* 启动时基*/
}

/**
 * @brief       定时器底层驱动，开启时钟，设置中断优先级
                会被HAL_TIM_Base_Init()调用
 * @param       htim  : 定时器句柄
 * @retval      无
 */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == BTIM_TIMX_INT)
    {
        BTIM_TIMX_INT_CLK_ENABLE();                     /* 使能TIMx时钟 */
        HAL_NVIC_SetPriority(BTIM_TIMX_INT_IRQn, 1, 3); /* 设置中断优先级 */
        HAL_NVIC_EnableIRQ(BTIM_TIMX_INT_IRQn);         /* 打开中断 */
    }
}

/**
 * @brief       基本定时器TIMX中断服务函数
 * @param       无
 * @retval      无
 */
void BTIM_TIMX_INT_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&g_timx_handle);                 /* 定时器回调函数 */
}

/**
 * @brief       定时器中断回调函数
 * @param       htim  : 定时器句柄
 * @retval      无
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == BTIM_TIMX_INT)
    {
        lv_tick_inc(1);         /* LVGL系统心跳 */
    }
}
