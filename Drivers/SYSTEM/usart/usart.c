/**
 ****************************************************************************************************
 * @file        usart.c
 * @author      ALIENTEK
 * @version     V1.1
 * @date        2023-02-28
 * @brief       串口初始化，支持Printf打印至串口
 * @license     MIT
 ****************************************************************************************************
 * @attention
 *
 *
 ****************************************************************************************************
 */

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"


/* 如果使用OS，引用下面的头文件 */
#if SYS_SUPPORT_OS
#include "os.h"          /* os 使用 */
#endif

/******************************************************************************************/
/* 加入以下代码以支持Printf()打印至串口，无需启用 MicroLIB */

#if 1
#if (__ARMCC_VERSION >= 6010050)                    /* AC6编译器 */
__asm(".global __use_no_semihosting\n\t");          /* 不使用半主机模式 */
__asm(".global __ARM_use_no_argv \n\t");            /* 无参数格式 */

#else
/* AC5环境下定义__FILE__，不使用半主机模式 */
#pragma import(__use_no_semihosting)

struct __FILE
{
    int handle;
    /* Whatever you require here. If the only file you are using is */
    /* standard output using printf() for debugging, no file handling */
    /* is required. */
};

#endif

int _ttywrch(int ch)
{
    ch = ch;
    return ch;
}

void _sys_exit(int x)
{
    x = x;
}

char *_sys_command_string(char *cmd, int len)
{
    return NULL;
}

FILE __stdout;

/* 重定向fputc，令printf打印至串口 */
int fputc(int ch, FILE *f)
{
    while ((USART_UX->SR & 0X40) == 0);               /* 等待上一个字符发送完成 */

    USART_UX->DR = (uint8_t)ch;                       /* 待发送字符写入DR寄存器 */
    return ch;
}
#endif
/***********************************************END*******************************************/
    
#if USART_EN_RX                                     /* 如果使能接收 */

/* 接收缓冲, 最大USART_REC_LEN个字节 */
uint8_t g_usart_rx_buf[USART_REC_LEN];

/*  接收状态
 *  bit15     接收完成标志位
 *  bit14     接收到0x0d
 *  bit13 - 0   接收到的有效字节数目
*/
uint16_t g_usart_rx_sta = 0;

uint8_t g_rx_buffer[RXBUFFERSIZE];    /* HAL使用的串口接收缓冲 */

UART_HandleTypeDef g_uart1_handle;    /* UART句柄 */

/**
 * @brief       串口X中断服务函数
 * @param       无
 * @retval      无
 */
void USART_UX_IRQHandler(void)
{ 
#if SYS_SUPPORT_OS                              /* 使用OS */
    OSIntEnter();    
#endif

    HAL_UART_IRQHandler(&g_uart1_handle);       /* 调用HAL库中断处理公用函数 */

#if SYS_SUPPORT_OS                              /* 使用OS */
    OSIntExit();
#endif
}


/**
 * @brief       串口X初始化函数
 * @param       baudrate: 波特率
 * @retval      无
 */
void usart_init(uint32_t baudrate)
{
    g_uart1_handle.Instance = USART_UX;                         /* USART1 */
    g_uart1_handle.Init.BaudRate = baudrate;                    /* 波特率 */
    g_uart1_handle.Init.WordLength = UART_WORDLENGTH_8B;        /* 8位数据 */
    g_uart1_handle.Init.StopBits = UART_STOPBITS_1;             /* 1位停止位 */
    g_uart1_handle.Init.Parity = UART_PARITY_NONE;              /* 无奇偶校验 */
    g_uart1_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;        /* 无硬件流控 */
    g_uart1_handle.Init.Mode = UART_MODE_TX_RX;                 /* 收发模式 */
    HAL_UART_Init(&g_uart1_handle);                             /* 初始化 */
    
    /* 准备进行中断接收 */
    HAL_UART_Receive_IT(&g_uart1_handle, (uint8_t *)g_rx_buffer, RXBUFFERSIZE);
}

/**
 * @brief       UART底层初始化函数
 * @param       huart: UART句柄类型指针
 * @note        此函数会被HAL_UART_Init()调用
 * @retval      无
 */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef gpio_init_struct;
    if (huart->Instance == USART_UX)                            /* 如果是UART1 */
    {
        USART_UX_CLK_ENABLE();                                  /* USART1 时钟使能 */
        USART_TX_GPIO_CLK_ENABLE();                             /* 发送Pin时钟使能 */
        USART_RX_GPIO_CLK_ENABLE();                             /* 接收Pin时钟使能 */

        gpio_init_struct.Pin = USART_TX_GPIO_PIN;               /* TX引脚 */
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;                /* 复用推挽输出 */
        gpio_init_struct.Pull = GPIO_PULLUP;                    /* 上拉 */
        gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;          /* 高速 */
        gpio_init_struct.Alternate = USART_TX_GPIO_AF;          /* 复用为USART1 */
        HAL_GPIO_Init(USART_TX_GPIO_PORT, &gpio_init_struct);   /* 初始化TX引脚 */

        gpio_init_struct.Pin = USART_RX_GPIO_PIN;               /* RX引脚 */
        gpio_init_struct.Alternate = USART_RX_GPIO_AF;          /* 复用为USART1 */
        HAL_GPIO_Init(USART_RX_GPIO_PORT, &gpio_init_struct);   /* 初始化RX引脚 */

#if USART_EN_RX
        HAL_NVIC_EnableIRQ(USART_UX_IRQn);                      /* 使能USART1中断通道 */
        HAL_NVIC_SetPriority(USART_UX_IRQn, 3, 3);              /* 设置中断优先级 */
#endif
    }
}

/**
 * @brief       RX传输回调
 * @param       huart: UART句柄类型指针
 * @retval      无
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART_UX)            /* USART1 */
    {
        if ((g_usart_rx_sta & 0x8000) == 0)     /* 接收未完成 */
        {
            if (g_usart_rx_sta & 0x4000)        /* 接收到了0x0d */
            {
                if (g_rx_buffer[0] != 0x0a) 
                {
                    g_usart_rx_sta = 0;         /* 接收错误,重新开始 */
                }
                else 
                {
                    g_usart_rx_sta |= 0x8000;   /* 标记接收完成 */
                }
            }
            else                                /* 未收到0X0D */
            {
                if (g_rx_buffer[0] == 0x0d)
                {
                    g_usart_rx_sta |= 0x4000;
                }
                else
                {
                    g_usart_rx_buf[g_usart_rx_sta & 0X3FFF] = g_rx_buffer[0] ;
                    g_usart_rx_sta++;
                    if (g_usart_rx_sta > (USART_REC_LEN - 1))
                    {
                        g_usart_rx_sta = 0;     /* 接收数据错误,重新开始接收 */
                    }
                }
            }
        }
        HAL_UART_Receive_IT(&g_uart1_handle, (uint8_t *)g_rx_buffer, RXBUFFERSIZE);
    }
}


#endif


 

 




