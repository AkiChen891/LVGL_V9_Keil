## 适用于STM32F429 阿波罗开发板的Keil工程文件
### 已支持的外设
- 基本定时器（TIM6）
- 按键
- LED
- SDRAM
- LCD（兼容LTDC）
- 触摸屏
- 客制化IIC（软件模拟方式）
- 24CXX存储IC驱动
- UART
- CAN
### 已支持的开发库
- LVGL v9.2
### 注意事项：
- 系统时钟：SYSCLK=90Mhz，APB1外设=45Mhz，APB1定时器=90Mhz，APB2外设=90MHz，APB2定时器=180Mhz
- UART波特率：115200，支持printf()重定向至外部串口
- CAN波特率：500kbps
- 栈大小：8KB
- 堆大小：8KB
### 文件架构
- Drivers：
    - BSP：板级支持包驱动
    - CMSIS：ARM提供的CMSIS代码，主要包括各种头文件和启动文件
    - STM32F4xx_HAL_Driver：ST提供的F4xx HAL库驱动代码
- Middlewares：
    - LVGL：LVGL库文件及驱动代码
- Output：
    编译输出临时文件夹
- Projects：
    项目文件夹
- User：
    主文件，外设启用设置文件，中断设置文件
 
