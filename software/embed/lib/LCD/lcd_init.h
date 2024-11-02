#ifndef __LCD_INIT_H
#define __LCD_INIT_H

#include <stdint.h>

#define USE_HORIZONTAL 2 // 设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏

#if USE_HORIZONTAL == 0 || USE_HORIZONTAL == 1
#define LCD_W 128
#define LCD_H 160

#else
#define LCD_W 160
#define LCD_H 128
#endif

//-----------------LCD端口定义----------------

// #define LCD_SCLK_Clr() gpio_bit_reset(GPIOC, GPIO_PIN_10) // SCL=SCLK
// #define LCD_SCLK_Set() gpio_bit_set(GPIOC, GPIO_PIN_10)

// #define LCD_MOSI_Clr() gpio_bit_reset(GPIOC, GPIO_PIN_12) // SDA=MOSI
// #define LCD_MOSI_Set() gpio_bit_set(GPIOC, GPIO_PIN_12)

#define LCD_RES_Clr() gpio_bit_reset(GPIOD, GPIO_PIN_8) // RES
#define LCD_RES_Set() gpio_bit_set(GPIOD, GPIO_PIN_8)

#define LCD_DC_Clr() gpio_bit_reset(GPIOD, GPIO_PIN_10) // DC
#define LCD_DC_Set() gpio_bit_set(GPIOD, GPIO_PIN_10)

#define LCD_CS_Clr() gpio_bit_reset(GPIOD, GPIO_PIN_11) // CS
#define LCD_CS_Set() gpio_bit_set(GPIOD, GPIO_PIN_11)

#define LCD_BLK_Clr() gpio_bit_reset(GPIOD, GPIO_PIN_12) // BLK
#define LCD_BLK_Set() gpio_bit_set(GPIOD, GPIO_PIN_12)

void LCD_Writ_Bus(uint8_t dat); // 模拟SPI时序
void LCD_WR_DATA8(uint8_t dat); // 写入一个字节
void LCD_WR_DATA(uint16_t dat); // 写入两个字节
void LCD_WR_REG(uint8_t dat);   // 写入一个指令
void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2,
                     uint16_t y2); // 设置坐标函数
void LCD_Init(void);               // LCD初始化
#endif
