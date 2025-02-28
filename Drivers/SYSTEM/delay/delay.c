/**
 ****************************************************************************************************
 * @file        delay.c
 * @author      ALIENTEK, recode by Chen Cheng
 * @version     V1.1
 * @date        2024-10-25
 * @brief       系统延时函数
 ****************************************************************************************************
 * @attention
 *
 ****************************************************************************************************
 */

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/delay/delay.h"
 
 
 static uint32_t g_fac_us = 0;       /* us延时倍乘数 */
 
 /* 如果该宏被定义，说明即将支持OS */
 #if SYS_SUPPORT_OS
 
 /* 公共头文件 */
 #include "os.h"
 
 /* 定义g_fac_ms变量，表示ms延时的倍乘数，代表每个节拍的ms数 */
 static uint16_t g_fac_ms = 0;
 
 /*
  *  当delay_us/delay_ms需要支持OS时，需要三个与OS相关的宏定义和函数来支持
  *  3个宏定义:
  *      delay_osrunning    :表示OS是否正在运行，以决定是否可以使用相关函数
  *      delay_ostickspersec:表示OS设定的时钟节拍，delay_init根据该参数来初始化systick
  *      delay_osintnesting :表示OS中断嵌套级别，delay_ms使用该参数来决定如何运行
  *  3个函数:
  *      delay_osschedlock  :锁定OS任务调度（禁止调度）
  *      delay_osschedunlock:解锁OS任务调度（开启调度）
  *      delay_ostimedly    :OS延时，可引起任务调度
  *
  *  本例程仅作UCOSII支持
  */
  
 /*  支持UCOSII */
 #define delay_osrunning     OSRunning           /* OS是否运行标记 */
 #define delay_ostickspersec OS_TICKS_PER_SEC    /* OS时钟节拍（每秒调度次数） */
 #define delay_osintnesting  OSIntNesting        /* 中断嵌套级别 */
 
 /**
  * @brief     us级延时时，关闭任务调度（防止任务打断us级延时）
  * @param     无  
  * @retval    无
  */  
 void delay_osschedlock(void)
 {
     OSSchedLock();                      /* UCOSII的方式，禁止调度 */
 }
 
 /**
  * @brief     us级延时时，恢复任务调度
  * @param     无
  * @retval    无
  */  
 void delay_osschedunlock(void)
 {
     OSSchedUnlock();                    /* UCOSII的方式，恢复调度 */
 }
 
 /**
  * @brief     us级延时，恢复任务调度
  * @param     ticks : 延时的节拍数
  * @retval    无
  */  
 void delay_ostimedly(uint32_t ticks)
 {
     OSTimeDly(ticks);                               /* UCOSII延时 */ 
 }
 
 /**
  * @brief     systick中断服务函数，使用OS时用到
  * @param     ticks : 延时节拍数  
  * @retval    无
  */  
 void SysTick_Handler(void)
 {
     if (delay_osrunning == OS_TRUE) /* OS运行时再执行调度处理 */
     {
         OS_CPU_SysTickHandler();  
     }
     HAL_IncTick();
 }
 #endif
 
 /**
  * @brief     延时函数初始化
  * @param     sysclk: 系统时钟（SYSCLK）频率，单位MHz
  * @retval    无
  */  
 void delay_init(uint16_t sysclk)
 {
 #if SYS_SUPPORT_OS                                      /* 如果需要支持OS */
     uint32_t reload;
 #endif
     g_fac_us = sysclk;                                  
 #if SYS_SUPPORT_OS                                      
     reload = sysclk;                                    
     reload *= 1000000 / delay_ostickspersec;            
                                                            
                                                          
     g_fac_ms = 1000 / delay_ostickspersec;              
     SysTick->CTRL |= 1 << 1;                            
     SysTick->LOAD = reload;                            
     SysTick->CTRL |= 1 << 0;                            
 #endif 
 }
 
 
 /**
  * @brief     延时函数（微妙）
  * @note      时钟摘取法
  * @param     nus: 延时值（微妙）
  * @note      取值范围：0 ~ (2^32 / fac_us)，fac_us一般等于sysclk频率
  * @retval    无
  */ 
 void delay_us(uint32_t nus)
 {
     uint32_t ticks;
     uint32_t told, tnow, tcnt = 0;
     uint32_t reload = SysTick->LOAD;        
     ticks = nus * g_fac_us;                 /* 需要的tick数 */
 
 #if SYS_SUPPORT_OS                          
     delay_osschedlock();                    
 #endif
 
     told = SysTick->VAL;                   
     while (1)
     {
         tnow = SysTick->VAL;
         if (tnow != told)
         {
             if (tnow < told)
             {
                 tcnt += told - tnow;        
             }
             else
             {
                 tcnt += reload - tnow + told;
             }
             told = tnow;
             if (tcnt >= ticks) 
             {
                 break;                      
             }
         }
     }
 
 #if SYS_SUPPORT_OS                          
     delay_osschedunlock();                  
 #endif
 } 
 
 /**
  * @brief     延时函数（毫秒）
  * @param     nms: 延时值（毫秒）
  * @note      取值范围：0 ~ (2^32 / fac_us / 1000) fac_us一般等于sysclk频率
  * @retval    无
  */
 void delay_ms(uint16_t nms)
 {
 #if SYS_SUPPORT_OS  
     if (delay_osrunning && delay_osintnesting == 0)     
     {
         if (nms >= g_fac_ms)                            
         {
             delay_ostimedly(nms / g_fac_ms);            
         }
 
         nms %= g_fac_ms;                                
     }
 #endif
 
     delay_us((uint32_t)(nms * 1000));                   
 }
 
 
 /**
  * @brief       HAL库默认延时
  * @note        HAL库延时函数使用systick，如果未启用systick中断会导致调用该延时后无法退出
  * @param       Delay : 待延时时间（毫秒）
  * @retval      无
  */
 void HAL_Delay(uint32_t Delay)
 {
     delay_ms(Delay);
 }
 
 
 
 
 
 
 
 
 
 
 