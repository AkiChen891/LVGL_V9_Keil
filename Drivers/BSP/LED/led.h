/**
  ******************************************************************************
  * @file           : led.h
  * @brief          : LED函数头文件
  ******************************************************************************
  * @attention
  *
  *     此文件应当在main.c中被include
  ******************************************************************************
  */
#ifndef __LED_H
#define __LED_H

#include "./SYSTEM/sys/sys.h"

void led_init();
void led_green(uint8_t flag);
void led_red(uint8_t flag);
void led_red_toggle();
void led_green_toggle();

#endif
