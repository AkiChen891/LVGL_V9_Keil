#include "lvgl.h"
#include "./BSP/CAN/can.h"
#include "./SYSTEM/sys/sys.h"

void create_welcome_scr(void);
void create_main_scr(void);
void lvgl_create_timers_1(void);

typedef struct DC_bus_parameters
{
    uint16_t voltage;               /* 直流母线电压 */
    uint16_t current;               /* 直流母线电流 */
    uint16_t power;                 /* 直流母线功率 */
    uint16_t excitation_current;    /* 励磁电流 */
}dc_bus;
