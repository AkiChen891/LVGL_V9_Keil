/**
 ****************************************************************************************************
 * @file        welcome_scr.c
 * @author      Aki
 * @version     V1.0
 * @date        2025-02-02
 * @brief       LVGL显示服务
 ****************************************************************************************************
 * @attention
 *              
 *
 ****************************************************************************************************
 */

 #include "lvgl.h"
 #include "string.h"
 #include "./SYSTEM/sys/sys.h"
 #include "./SYSTEM/usart/usart.h"
 #include "./BSP/CAN/can.h"
 #include "lv_dcbus.h"

 
 /************************ 变量声明 ********************** */
 
 static lv_obj_t *welcome_scr;
 static lv_obj_t *welcome_label;
 static lv_obj_t *btn_welcome;
 static lv_obj_t *btn_welcome_label;
 static lv_obj_t *main_scr;
 static lv_obj_t *main_label;
 static lv_obj_t *dc_bus_voltage_page;
 static lv_obj_t *connection_status_page;
 static lv_obj_t *CAN_label;
 static lv_timer_t *timer_1;
 static lv_timer_t *timer_2;
 static lv_obj_t *voltage_label;
 static lv_obj_t *dcbus_current_label;
 static lv_obj_t *power_label;
 static lv_obj_t *excitation_current_label;
 static lv_obj_t *CAN_error_label;
 
 /************************ 函数声明 ********************** */
 
 void create_welcome_scr(void);
 void create_main_scr(void);
 void create_dc_bus_voltage_page(void);
 void create_connection_status_page(void);
 void lvgl_create_timers_1(void);
 void lvgl_create_timers_2(void);
 void btn_welcome_event_cb(lv_event_t * e);
 void btn_to_dcbus_voltage_page_cb(lv_event_t *e);
 void btn_to_system_connection_status_cb(lv_event_t *e);
 void btn_from_dc_bus_to_main_page_cb(lv_event_t *e);
 void lvgl_timer_1_cb(lv_timer_t *timer);
 void btn_to_main_page_cb(lv_event_t *e);
 void lvgl_timer_2_cb(lv_timer_t *timer);
 
 /************************ 结构体声明 ********************** */
 
 
 HAL_CAN_StateTypeDef CAN_last_state = HAL_CAN_STATE_RESET;   /* CAN 总线状态 */
 
 
 /************************ 页面相关函数  ********************** */
 
 /* 创建欢迎屏幕（开机界面） */
 void create_welcome_scr(void) {
     /* 创建屏幕父类对象 */
     welcome_scr = lv_obj_create(NULL);
     lv_obj_set_size(welcome_scr, 1024, 600);
 
     /* 创建"Welcome"label对象 */
     welcome_label = lv_label_create(welcome_scr);
     lv_label_set_text(welcome_label, "Welcome");
     static lv_style_t style_welcome_label;
     lv_style_init(&style_welcome_label);
     lv_style_set_text_font(&style_welcome_label, &lv_font_montserrat_40);
     lv_obj_add_style(welcome_label, &style_welcome_label, 0);
     lv_obj_align(welcome_label, LV_ALIGN_CENTER, 0, 0);
 
     /* 创建 "Start" button对象 */
     btn_welcome = lv_btn_create(welcome_scr);
     lv_obj_align(btn_welcome, LV_ALIGN_CENTER, 0, 100);
     lv_obj_set_size(btn_welcome, 150, 50);
 
     /* 创建"Start" button中的字符 */
     btn_welcome_label = lv_label_create(btn_welcome);
     lv_label_set_text(btn_welcome_label, "Start");
     lv_obj_align(btn_welcome_label, LV_ALIGN_CENTER, 0, 0);
     static lv_style_t style_btn_welcome_label;
     lv_style_init(&style_btn_welcome_label);
     lv_style_set_text_font(&style_btn_welcome_label, &lv_font_montserrat_32);
     lv_obj_add_style(btn_welcome_label, &style_btn_welcome_label, 0);
     lv_obj_clear_flag(btn_welcome, LV_OBJ_FLAG_SCROLLABLE);
     lv_obj_set_style_bg_color(btn_welcome, lv_palette_main(LV_PALETTE_GREY),NULL);
     lv_obj_add_event_cb(btn_welcome, btn_welcome_event_cb, LV_EVENT_CLICKED, NULL);
     
     /* 加载屏幕 */
     lv_scr_load(welcome_scr);
 }
 
 /* 创建 "main" 界面 */
 void create_main_scr(void) {
     /* 判断该界面是否已创建 */
     if (main_scr && lv_obj_is_valid(main_scr)) {
         return; 
     }
 
     /* 创建父类对象 */
     main_scr = lv_obj_create(NULL);
     lv_obj_set_size(main_scr, 1024, 600);
 
     /* 创建标题label */
     main_label = lv_label_create(main_scr);
     lv_label_set_text(main_label, "MVDC IPS Fault Detection System"); 
     static lv_style_t style_main_label;
     lv_style_init(&style_main_label);
     lv_style_set_text_font(&style_main_label, &lv_font_montserrat_32);
     lv_obj_add_style(main_label, &style_main_label, 0);
     lv_obj_align(main_label, LV_ALIGN_TOP_MID, 0, 0);
 
     /* 创建 "Next Page" button */
     lv_obj_t *btn_to_dcbus_voltage_page = lv_btn_create(main_scr);
     lv_obj_align(btn_to_dcbus_voltage_page, LV_ALIGN_TOP_RIGHT, 0, 0);
     lv_obj_set_size(btn_to_dcbus_voltage_page, 120, 40);
     
     /* 创建 "Next Page" button 上显示的label对象 */
     lv_obj_t *btn_to_dcbus_voltage_page_label = lv_label_create(btn_to_dcbus_voltage_page);
     lv_label_set_text(btn_to_dcbus_voltage_page_label, "Next");
     lv_obj_align(btn_to_dcbus_voltage_page_label, LV_ALIGN_CENTER, 0, 0);
     static lv_style_t style_to_dcbus_voltage_page_label;
     lv_style_init(&style_to_dcbus_voltage_page_label);
     lv_style_set_text_font(&style_to_dcbus_voltage_page_label, &lv_font_montserrat_24);
     lv_obj_add_style(btn_to_dcbus_voltage_page_label, &style_to_dcbus_voltage_page_label, 0);
     lv_obj_clear_flag(btn_to_dcbus_voltage_page, LV_OBJ_FLAG_SCROLLABLE);
     lv_obj_set_style_bg_color(btn_to_dcbus_voltage_page, lv_palette_main(LV_PALETTE_GREY),NULL);
     lv_obj_add_event_cb(btn_to_dcbus_voltage_page, btn_to_dcbus_voltage_page_cb, LV_EVENT_CLICKED, NULL);
 
     /* 创建 "system connection status" button 对象 */
     lv_obj_t *btn_to_connection_status_page = lv_btn_create(main_scr);
     lv_obj_align(btn_to_connection_status_page, LV_ALIGN_TOP_RIGHT, 0, 50);
     lv_obj_set_size(btn_to_connection_status_page, 120, 40);
 
     /* 创建 "system connection status" button 上显示的label对象 */
     lv_obj_t *btn_to_connection_status_page_label = lv_label_create(btn_to_connection_status_page);
     lv_label_set_text(btn_to_connection_status_page_label, "STATUS");
     lv_obj_align(btn_to_connection_status_page_label, LV_ALIGN_CENTER, 0, 0);
     static lv_style_t style_to_connection_status_page_label;
     lv_style_init(&style_to_connection_status_page_label);
     lv_style_set_text_font(&style_to_connection_status_page_label, &lv_font_montserrat_24);
     lv_obj_add_style(btn_to_connection_status_page_label, &style_to_connection_status_page_label, 0);
     lv_obj_clear_flag(btn_to_connection_status_page, LV_OBJ_FLAG_SCROLLABLE);
     lv_obj_set_style_bg_color(btn_to_connection_status_page, lv_palette_main(LV_PALETTE_GREY),NULL);
     lv_obj_add_event_cb(btn_to_connection_status_page, btn_to_system_connection_status_cb, LV_EVENT_CLICKED, NULL);
 
     /* 仅创建对象，不加载界面 */
 }
 
 /* 创建 DC bus voltage 界面 */
 void create_dc_bus_voltage_page(void) {
     /* 创建父类对象 */
     dc_bus_voltage_page = lv_obj_create(NULL);
     lv_obj_set_size(dc_bus_voltage_page, 1024, 600);
 
     /* 创建标题 label */
     lv_obj_t * dc_bus_voltage_title_label = lv_label_create(dc_bus_voltage_page);
     lv_label_set_text(dc_bus_voltage_title_label, "DC Bus Voltage Detection");
     static lv_style_t style_dc_bus_voltage_title_label;
     lv_style_init(&style_dc_bus_voltage_title_label);
     lv_style_set_text_font(&style_dc_bus_voltage_title_label, &lv_font_montserrat_32);
     lv_obj_add_style(dc_bus_voltage_title_label, &style_dc_bus_voltage_title_label, 0);
     lv_obj_align(dc_bus_voltage_title_label, LV_ALIGN_TOP_MID, 0, 0);
 
     /* 创建电压label */
     voltage_label = lv_label_create(dc_bus_voltage_page);
     lv_label_set_text(voltage_label, "Voltage: ");
     lv_obj_align(voltage_label, LV_ALIGN_TOP_LEFT, 50, 60);
     static lv_style_t style_voltage_label;
     lv_style_init(&style_voltage_label);
     lv_style_set_text_font(&style_voltage_label, &lv_font_montserrat_28);
     lv_obj_add_style(voltage_label, &style_voltage_label, 0);
 
     /* 创建电流label */
     dcbus_current_label = lv_label_create(dc_bus_voltage_page);
     lv_label_set_text(dcbus_current_label, "Current: ");
     lv_obj_align(dcbus_current_label, LV_ALIGN_TOP_LEFT, 50, 100);
     static lv_style_t style_dcbus_current_label;
     lv_style_init(&style_dcbus_current_label);
     lv_style_set_text_font(&style_dcbus_current_label, &lv_font_montserrat_28);
     lv_obj_add_style(dcbus_current_label, &style_dcbus_current_label, 0);
 
     /* 创建功率label */
     power_label = lv_label_create(dc_bus_voltage_page);
     lv_label_set_text(power_label, "Power: ");
     lv_obj_align(power_label, LV_ALIGN_TOP_LEFT, 50, 140);
     static lv_style_t style_power_label;
     lv_style_init(&style_power_label);
     lv_style_set_text_font(&style_power_label, &lv_font_montserrat_28);
     lv_obj_add_style(power_label, &style_power_label, 0);
 
     /* 创建励磁电流label */
     excitation_current_label = lv_label_create(dc_bus_voltage_page);
     lv_label_set_text(excitation_current_label, "Excitation Current: ");
     lv_obj_align(excitation_current_label, LV_ALIGN_TOP_LEFT, 50, 180);
     static lv_style_t style_excitation_current_label;
     lv_style_init(&style_excitation_current_label);
     lv_style_set_text_font(&style_excitation_current_label, &lv_font_montserrat_28);
     lv_obj_add_style(excitation_current_label, &style_excitation_current_label, 0);
 
     /* 创建返回main页面的button对象 */
     lv_obj_t *btn_from_dc_bus_to_main_page = lv_btn_create(dc_bus_voltage_page);
     lv_obj_align(btn_from_dc_bus_to_main_page, LV_ALIGN_TOP_LEFT, 0, 0);
     lv_obj_set_size(btn_from_dc_bus_to_main_page, 120, 40);
 
     /* 向button对象添加显示label */
     lv_obj_t *btn_from_dc_bus_to_main_page_label = lv_label_create(btn_from_dc_bus_to_main_page);
     lv_label_set_text(btn_from_dc_bus_to_main_page_label, "Back");
     lv_obj_align(btn_from_dc_bus_to_main_page_label, LV_ALIGN_CENTER, 0, 0);
     static lv_style_t style_btn_from_dc_bus_to_main_page_label;
     lv_style_init(&style_btn_from_dc_bus_to_main_page_label);
     lv_style_set_text_font(&style_btn_from_dc_bus_to_main_page_label, &lv_font_montserrat_24);
     lv_obj_add_style(btn_from_dc_bus_to_main_page_label, &style_btn_from_dc_bus_to_main_page_label, 0);
     lv_obj_clear_flag(btn_from_dc_bus_to_main_page, LV_OBJ_FLAG_SCROLLABLE);
     lv_obj_set_style_bg_color(btn_from_dc_bus_to_main_page, lv_palette_main(LV_PALETTE_GREY),NULL);
     lv_obj_add_event_cb(btn_from_dc_bus_to_main_page, btn_from_dc_bus_to_main_page_cb, LV_EVENT_CLICKED, NULL);
 
     /* 仅创建，不加载 */
     /* 创建定时器timer_2 */
     lvgl_create_timers_2();
 }
 
 /* 创建connection status页面 */
 void create_connection_status_page(void) {
     /* 创建父类对象 */
     connection_status_page = lv_obj_create(NULL);
     lv_obj_set_size(connection_status_page, 1024, 600);
 
     /* 创建标题label */
     lv_obj_t * connection_status_title_label = lv_label_create(connection_status_page);
     lv_label_set_text(connection_status_title_label, "System Connection Status");
     static lv_style_t style_connection_status_title_label;
     lv_style_init(&style_connection_status_title_label);
     lv_style_set_text_font(&style_connection_status_title_label, &lv_font_montserrat_32);
     lv_obj_add_style(connection_status_title_label, &style_connection_status_title_label, 0);
     lv_obj_align(connection_status_title_label, LV_ALIGN_TOP_MID, 0, 0);
 
     /* 创建 CAN bus status label */
     CAN_label = lv_label_create(connection_status_page);
     lv_label_set_text(CAN_label, "CAN: Getting status...");
     static lv_style_t style_CAN_label;
     lv_style_init(&style_CAN_label);
     lv_style_set_text_font(&style_CAN_label, &lv_font_montserrat_28);
     lv_obj_add_style(CAN_label, &style_CAN_label, 0);
     lv_obj_align(CAN_label, LV_ALIGN_TOP_LEFT, 50, 60);
 
     /* 创建"Back"button */
     lv_obj_t *btn_to_main_page = lv_btn_create(connection_status_page);
     lv_obj_align(btn_to_main_page, LV_ALIGN_TOP_LEFT, 0, 0);
     lv_obj_set_size(btn_to_main_page, 120, 40);
 
     /* 创建"Back"button 显示label */
     lv_obj_t *btn_to_main_page_label = lv_label_create(btn_to_main_page);
     lv_label_set_text(btn_to_main_page_label, "Back");
     lv_obj_align(btn_to_main_page_label, LV_ALIGN_CENTER, 0, 0);
     static lv_style_t style_to_main_page_label;
     lv_style_init(&style_to_main_page_label);
     lv_style_set_text_font(&style_to_main_page_label, &lv_font_montserrat_24);
     lv_obj_add_style(btn_to_main_page_label, &style_to_main_page_label, 0);
     lv_obj_clear_flag(btn_to_main_page, LV_OBJ_FLAG_SCROLLABLE);
     lv_obj_set_style_bg_color(btn_to_main_page, lv_palette_main(LV_PALETTE_GREY),NULL);
     lv_obj_add_event_cb(btn_to_main_page, btn_to_main_page_cb, LV_EVENT_CLICKED, NULL);
 
     /* 仅创建，不加载 */
     /* 创建定时器timer_1 */
     lvgl_create_timers_1();
 }
 
 
 /************************ 定时器创建 ********************** */
 
 /* 定时器1，用于查询总线状态 */
 void lvgl_create_timers_1(void)
 {
     /* 查询是否有旧的定时器 */
     if (timer_1 != NULL) {
         lv_timer_del(timer_1);  /* 删除旧的定时器 */
         timer_1 = NULL;         /* 清空指针 */
     }
 
     /* 创建新的定时器，500ms一次回调 */
     timer_1 = lv_timer_create(lvgl_timer_1_cb, 500, NULL);
     /* 更新CAN状态 */
     CAN_last_state = HAL_CAN_STATE_RESET;
 }
 
 /* 定时器2，用于DC bus voltage页面数据更新 */
 void lvgl_create_timers_2(void)
 {
     if (timer_2 != NULL) {
         lv_timer_del(timer_2);
         timer_2 = NULL;
     }
 
     /* 创建新的定时器，100ms一次回调 */
     timer_2 = lv_timer_create(lvgl_timer_2_cb, 100, NULL);
 }
 
 
 /************************ 回调函数 ********************** */
 
 /* Welcome page "Start" button callback */
 void btn_welcome_event_cb(lv_event_t * e){
     if(lv_event_get_code(e) == LV_EVENT_CLICKED){
         lv_obj_t *act_scr = lv_scr_act();
         create_main_scr();
         lv_scr_load(main_scr);
         //lv_obj_clean(act_scr);
         lv_obj_del(act_scr);
     }
 }
 
 
 /* Main page "Next" button callback */
 void btn_to_dcbus_voltage_page_cb(lv_event_t *e){
     if (lv_event_get_code(e) == LV_EVENT_CLICKED){
         lv_obj_t *act_scr = lv_scr_act();
         create_dc_bus_voltage_page();
         lv_scr_load(dc_bus_voltage_page);
         //lv_obj_clean(act_scr);
         lv_obj_del(act_scr);
     }
 }
 
 /* Main page "Status" button callback */
 void btn_to_system_connection_status_cb(lv_event_t *e){
     if (lv_event_get_code(e) == LV_EVENT_CLICKED){
         lv_obj_t *act_scr = lv_scr_act();
         create_connection_status_page();
         lv_scr_load(connection_status_page);
         //lv_obj_clean(act_scr);
         lv_obj_del(act_scr);
     }
 }
 
 /* Connection status page "Back" button callback */
 void btn_to_main_page_cb(lv_event_t *e){
     if (lv_event_get_code(e) == LV_EVENT_CLICKED){
         if (timer_1 != NULL) {
             lv_timer_del(timer_1);  
             timer_1 = NULL;         
         }
         lv_obj_t *act_scr = lv_scr_act();
         create_main_scr();        
         lv_scr_load(main_scr);
         //lv_obj_clean(act_scr);
         lv_obj_del(act_scr);
     }
 }
 
 /* DC bus page "Back" button callback */
 void btn_from_dc_bus_to_main_page_cb(lv_event_t *e)
 {
     if (lv_event_get_code(e) == LV_EVENT_CLICKED)
     {
         if (timer_2 != NULL) {
             lv_timer_del(timer_2);  
             timer_2 = NULL;         
         }
         lv_obj_t *act_scr = lv_scr_act();
         create_main_scr();
         lv_scr_load(main_scr);
         //lv_obj_clean(act_scr);
         lv_obj_del(act_scr);
     }
 }
 
 /* Timer_1 回调函数 */
 void lvgl_timer_1_cb(lv_timer_t *timer)
 {
     extern CAN_HandleTypeDef g_canx_handle;
     HAL_CAN_StateTypeDef can_state = HAL_CAN_GetState(&g_canx_handle);
 
     if (can_state != CAN_last_state) {
         CAN_last_state = can_state;     /* 更新CAN状态 */
 
         switch (can_state) {
             case HAL_CAN_STATE_RESET:
                 lv_label_set_text_fmt(CAN_label, "CAN: RESET");
                 break;
 
             case HAL_CAN_STATE_READY:
                 lv_label_set_text_fmt(CAN_label, "CAN: READY");
                 break;
 
             case HAL_CAN_STATE_LISTENING:
                 lv_label_set_text_fmt(CAN_label, "CAN: LISTENING...");
                 break;
 
             case HAL_CAN_STATE_SLEEP_PENDING:
                 lv_label_set_text_fmt(CAN_label, "CAN: SLEEP PENDING...");
                 break;
 
             case HAL_CAN_STATE_SLEEP_ACTIVE:
                 lv_label_set_text_fmt(CAN_label, "CAN: SLEEP ACTIVE");
                 break;
 
             case HAL_CAN_STATE_ERROR:
                 lv_label_set_text_fmt(CAN_label, "CAN: ERROR");
                 break;
 
             default:
                 lv_label_set_text_fmt(CAN_label, "CAN: UNKNOWN STATE");
                 break;
         }
     }
 }
 
 /* Timer_2 回调函数 */
 extern uint8_t canbuf[4];
 void lvgl_timer_2_cb(lv_timer_t *timer)
 {
     /* 更新数据至label */
     lv_label_set_text_fmt(voltage_label, "Voltage: %dV", canbuf[0]);
     lv_label_set_text_fmt(dcbus_current_label, "Current: %dA", canbuf[1]);
     lv_label_set_text_fmt(power_label, "Power: %dW", canbuf[2]);
     lv_label_set_text_fmt(excitation_current_label, "Excitation Current: %dA", canbuf[3]);
 }