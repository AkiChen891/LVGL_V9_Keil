/**
 ****************************************************************************************************
 * @file        can.c
 * @author      ALIENTEK
 * @version     V1.0
 * @date        2022-04-20
 * @brief       CAN 驱动
 ****************************************************************************************************
 * @attention
 *
 *
 ****************************************************************************************************
 */

#include "./BSP/CAN/can.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"


CAN_HandleTypeDef       g_canx_handle;                  /* CAN1句柄 */
CAN_TxHeaderTypeDef     g_canx_txheade;                 /* 发送句柄 */
CAN_RxHeaderTypeDef     g_canx_rxheade;                 /* 接收句柄 */

/**
 * @brief       CAN初始化
 * @param       tsjw:SJW同步段             范围:CAN_SJW_1TQ~CAN_SJW_4TQ
 * @param       tbs2:时间段2               范围:CAN_BS2_1TQ~CAN_BS2_8TQ;
 * @param       tbs1:时间段1               范围:CAN_BS1_1TQ~CAN_BS1_16TQ
 * @param       brp :分频系数，范围:1 - 1024; tq=(brp)*tpclk1
 * @note        波特率 = Fpclk1/((tbs1+tbs2+1)*brp); 
 *              计算网址：http://www.bittiming.can-wiki.info/
 * @param       mode:CAN_MODE_NORMAL,普通模式;CAN_MODE_LOOPBACK,回环模式;
 * @retval      返回值:0,初始化成功; 其他,初始化失败; 
 */
uint8_t can_init(uint32_t tsjw, uint32_t tbs2, uint32_t tbs1, uint16_t brp, uint32_t mode) 
{
    g_canx_handle.Instance = CAN1; 
    g_canx_handle.Init.Prescaler = brp;                 
    g_canx_handle.Init.Mode = mode;                     
    g_canx_handle.Init.SyncJumpWidth = tsjw;            
    g_canx_handle.Init.TimeSeg1 = tbs1;                 
    g_canx_handle.Init.TimeSeg2 = tbs2;                 
    g_canx_handle.Init.TimeTriggeredMode = DISABLE;     /* 非时间触发通信模式 */
    g_canx_handle.Init.AutoBusOff = DISABLE;            /* 软件自动离线管理 */
    g_canx_handle.Init.AutoWakeUp = DISABLE;            /* 睡眠模式下自动唤醒*/
    g_canx_handle.Init.AutoRetransmission = ENABLE;     /* 报文自动重传 */
    g_canx_handle.Init.ReceiveFifoLocked = DISABLE;     /* FIFO不锁定，新消息覆盖旧消息  */
    g_canx_handle.Init.TransmitFifoPriority = DISABLE;  /* 优先级由报文ID决定  */

    if (HAL_CAN_Init(&g_canx_handle) != HAL_OK) 
    {
        return 1;   
    }

#if CAN1_RX0_INT_ENABLE
    __HAL_CAN_ENABLE_IT(&g_canx_handle, CAN_IT_RX_FIFO0_MSG_PENDING); /* FIFO0消息挂起中断 */
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 1, 0);                        /* 设置中断优先级 */
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);                                /* 使能中断 */
#endif

    /* 过滤器设置 */
    CAN_FilterTypeDef sFilterConfig;

    sFilterConfig.FilterIdHigh = 0X0000;                        /* 32位ID */
    sFilterConfig.FilterIdLow = 0X0000;
    sFilterConfig.FilterMaskIdHigh = 0X0000;                    /* 32位掩码 */
    sFilterConfig.FilterMaskIdLow = 0X0000;  
    sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;      /* 过滤器0关联至FIFO0 */
    sFilterConfig.FilterBank = 0;                               /* 过滤器0 */
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterActivation = CAN_FILTER_ENABLE;         /* 激活过滤器0 */
    sFilterConfig.SlaveStartFilterBank = 14;

     /* 过滤器初始化 */
    if (HAL_CAN_ConfigFilter(&g_canx_handle, &sFilterConfig) != HAL_OK) 
    {
        return 2;                                              
    }

    /* 启动CAN */
    if (HAL_CAN_Start(&g_canx_handle) != HAL_OK)
    {
        return 3;
    }

    return 0;
}

/**
 * @brief       CAN底层驱动，引脚配置，时钟配置，中断配置
 * @note        会被HAL_CAN_Init()调用
 * @param       hcan:CAN句柄
 * @retval      无
 */
void HAL_CAN_MspInit(CAN_HandleTypeDef *hcan)
{
    if (CAN1 == hcan->Instance)
    {
        GPIO_InitTypeDef gpio_init_struct;

        __HAL_RCC_CAN1_CLK_ENABLE();                        /* 使能CAN1时钟 */
        CAN_RX_GPIO_CLK_ENABLE();                           /* 使能CAN_RX引脚时钟 */
        CAN_TX_GPIO_CLK_ENABLE();                           /* 使能CAN_TX引脚时钟 */

        gpio_init_struct.Pin = CAN_RX_GPIO_PIN;             /* PA11 */
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;            /* 推挽复用 */
        gpio_init_struct.Pull = GPIO_PULLUP;                /* 上拉 */
        gpio_init_struct.Speed = GPIO_SPEED_FAST;           /* 高速 */
        gpio_init_struct.Alternate = GPIO_AF9_CAN1;         /* 复用为CAN1 */
        HAL_GPIO_Init(CAN_RX_GPIO_PORT, &gpio_init_struct); /* 初始化IO */

        gpio_init_struct.Pin = CAN_TX_GPIO_PIN;             /* PA12 */
        HAL_GPIO_Init(CAN_TX_GPIO_PORT, &gpio_init_struct); /* CAN_RX必须设置成输入模式 */
    }
}

#if CAN1_RX0_INT_ENABLE                                     /* 使能RX0中断 */

/**
 * @brief       CAN中断服务函数
 * @param       无
 * @retval      无;
 */
void CAN1_RX0_IRQHandler(void)
{
    HAL_CAN_IRQHandler(&CAN1_Handler);                      /* 此函数调用CAN_Receive_IT()接收数据*/
}

/**
 * @brief       CAN接收中断回调(Legacy CAN)
 * @note        此函数被ᱻCAN_Receive_IT()调用
 * @param       hcan:CAN句柄
 * @retval      无;
 */
void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef* hcan)
{
    int i = 0;

    /* CAN_Receive_IT()会关闭FIFO0挂号中断，这里需重新打开 */
    __HAL_CAN_ENABLE_IT(&CAN1_Handler, CAN_IT_FMP0);        /* 重启FIFO0 */
    printf("id:%d\r\n", g_canx_txheade.StdId);
    printf("ide:%d\r\n", g_canx_txheade.IDE);
    printf("rtr:%d\r\n", g_canx_txheade.RTR);
    printf("len:%d\r\n", g_canx_txheade.DLC);

    for (i = 0; i < 8; i++)
    {
        printf("rxbuf[%d]:%d\r\n", i, CAN1_Handler.pRxMsg -> Data[i]);
    }
}

#endif

/**
 * @brief       CAN 发送一组数据
 * @note        发送格式：标准11位ID，数据帧
 * @param       id   : 11位标准ID
 * @param       msg  : 数据指针
 * @param       len  : 数据长度，最大8字节
 * @retval      发送状态 0, 成功; 1, 失败;
 */
uint8_t can_send_msg(uint32_t id, uint8_t *msg, uint8_t len)
{
    uint8_t waittime = 0;
    
    /* 填充发送句柄 */
    uint32_t TxMailbox = CAN_TX_MAILBOX0;
    g_canx_txheade.StdId = id;                 /* 标准标识符 */
    g_canx_txheade.ExtId = id;                 /* 扩展标识符(29位) */
    g_canx_txheade.IDE = CAN_ID_STD;           /* 使用标准帧 */
    g_canx_txheade.RTR = CAN_RTR_DATA;         /* 数据帧 */
    g_canx_txheade.DLC = len;

    if (HAL_CAN_AddTxMessage(&g_canx_handle, &g_canx_txheade, msg, &TxMailbox) != HAL_OK) /* 发送消息 */
    {
        return 1;
    }
    while(HAL_CAN_GetTxMailboxesFreeLevel(&g_canx_handle) != 3)                            /* 等待发送完成（所有邮箱清空） */
    {
        waittime++;
        
        if( waittime > 30)
        {
            return 1;
        }
        delay_ms(100);
    }
    return 0;
}

/**
 * @brief       CAN 接收数据查询
 * @note        消息格式: 标准ID，数据帧
 * @param       id      : 待查询ID（11位）
 * @param       buf     : 数据缓存区地址
 * @retval      接收结果
 *   @arg       0   , 未接收到数据;
 *   @arg       其他, 接收的数据长度
 */
uint8_t can_receive_msg(uint32_t id, uint8_t *buf)
{
    if (HAL_CAN_GetRxFifoFillLevel(&g_canx_handle, CAN_RX_FIFO0) == 0)                                             
    {
        return 0;
    }

    if (HAL_CAN_GetRxMessage(&g_canx_handle, CAN_RX_FIFO0, &g_canx_rxheade, buf) != HAL_OK)                         
    {
        return 0;
    }

    if (g_canx_rxheade.StdId != id || g_canx_rxheade.IDE != CAN_ID_STD || g_canx_rxheade.RTR != CAN_RTR_DATA)      
    {
        return 0;
    }

    return g_canx_rxheade.DLC;
}


