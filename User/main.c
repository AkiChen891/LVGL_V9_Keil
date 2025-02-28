/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ALIENTEK
 * @version     V1.0
 * @date        2022-4-20
 * @brief       空白主文件，已集成LVGL v9.2
 * @license     MIT
 ****************************************************************************************************
 * @attention
 *          系统时钟设置为：
 *              SYSCLK      180Mhz
 *              APB1外设    45Mhz
 *              APB1时钟    90Mhz
 *              APB2外设    90Mhz
 *              APB2时钟    180Mhz
 ****************************************************************************************************
 */

#include "./SYSTEM/sys/sys.h"               /* 系统底层驱动 */
#include "./SYSTEM/usart/usart.h"           /* 串口驱动 */
#include "./SYSTEM/delay/delay.h"           /* 延时函数驱动 */
#include "./BSP/LED/led.h"                  /* LED驱动 */
#include "./BSP/LCD/lcd.h"                  /* LCD（LTDC）驱动 */
#include "./BSP/KEY/key.h"                  /* 按键驱动 */
#include "./BSP/TOUCH/touch.h"              /* 触摸屏驱动 */
#include "./BSP/SDRAM/sdram.h"              /* SDRAM驱动 */
#include "./USMART/usmart.h"                /* USMART驱动 */
#include "./BSP/CAN/can.h"                  /* CAN驱动 */
#include "./BSP/TIMER/btim.h"               /* 基本定时器驱动 */
#include "lvgl.h"                           /* LVGL库 */
#include "lv_port_disp.h"                   /* LVGL显示设备注册 */
#include "lv_port_indev.h"                  /* LVGL输入设备注册 */
#include "./LVGL/lvgl_app/lv_dcbus.h"       /* LVGL服务函数 */

uint8_t canbuf[4];      /* CAN全局缓冲区 */

int main(void)
{
    uint8_t key;
    uint8_t i = 0, t = 0;
    uint8_t cnt = 0;
    uint8_t rxlen = 0;
    uint8_t res;

    HAL_Init();                                       /* 初始化HAL库 */
    sys_stm32_clock_init(360, 25, 2, 8);              /* 系统时钟初始化 */
    delay_init(180);                                  /* 延时初始化，SYSCLK=180Mhz */
	btim_timx_int_init(8999, 9);                      /* TIM6初始化，ARR=9000-1，PSC=10-1 */
    usart_init(115200);                               /* USART初始化，波特率115200 */
    led_init();                                       /* LED初始化 */
    sdram_init();                                     /* SDRAM初始化 */
    //lcd_init();                                       /* LCD初始化 */
    lv_init();                                        /* LVGL初始化 */
    lv_port_disp_init();                              /* LVGL注册显示设备 */
		lv_port_indev_init();
    //key_init();                                       /* 按键初始化 */
    can_init(CAN_SJW_1TQ, CAN_BS2_6TQ, CAN_BS1_8TQ, 6, CAN_MODE_NORMAL);      /* CAN初始化，正常模式，波特率500kbps */

    create_welcome_scr();

    while (1)
    {
        rxlen = can_receive_msg(0x12, canbuf);                                              /* CAN ID = 0x12, 接收消息 */
	
        lv_task_handler();
        delay_ms(5);
    }
}

