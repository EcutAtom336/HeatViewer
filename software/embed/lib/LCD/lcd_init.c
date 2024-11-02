#include "lcd_init.h"
//
#include "FreeRTOS.h"
#include "task.h"
//
#include "gd32f4xx.h"
//
#include <stddef.h>

/******************************************************************************
      函数说明：LCD串行数据写入函数
      入口数据：dat  要写入的串行数据
      返回值：  无
******************************************************************************/
void LCD_Writ_Bus(uint8_t dat) {
  LCD_CS_Clr();
  spi_i2s_data_transmit(SPI1, dat);
  while (spi_i2s_flag_get(SPI1, SPI_FLAG_TRANS) == SET);
  LCD_CS_Set();
}

/******************************************************************************
      函数说明：LCD写入数据
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA8(uint8_t dat) { LCD_Writ_Bus(dat); }

/******************************************************************************
      函数说明：LCD写入数据
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA(uint16_t dat) {
  LCD_Writ_Bus(dat >> 8);
  LCD_Writ_Bus(dat);
}

/******************************************************************************
      函数说明：LCD写入命令
      入口数据：dat 写入的命令
      返回值：  无
******************************************************************************/
void LCD_WR_REG(uint8_t dat) {
  LCD_DC_Clr();  // 写命令
  LCD_Writ_Bus(dat);
  LCD_DC_Set();  // 写数据
}

/******************************************************************************
      函数说明：设置起始和结束地址
      入口数据：x1,x2 设置列的起始和结束地址
                y1,y2 设置行的起始和结束地址
      返回值：  无
******************************************************************************/
void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
  if (USE_HORIZONTAL == 0) {
    LCD_WR_REG(0x2a);  // 列地址设置
    LCD_WR_DATA(x1 + 2);
    LCD_WR_DATA(x2 + 2);
    LCD_WR_REG(0x2b);  // 行地址设置
    LCD_WR_DATA(y1 + 1);
    LCD_WR_DATA(y2 + 1);
    LCD_WR_REG(0x2c);  // 储存器写
  } else if (USE_HORIZONTAL == 1) {
    LCD_WR_REG(0x2a);  // 列地址设置
    LCD_WR_DATA(x1 + 2);
    LCD_WR_DATA(x2 + 2);
    LCD_WR_REG(0x2b);  // 行地址设置
    LCD_WR_DATA(y1 + 1);
    LCD_WR_DATA(y2 + 1);
    LCD_WR_REG(0x2c);  // 储存器写
  } else if (USE_HORIZONTAL == 2) {
    LCD_WR_REG(0x2a);  // 列地址设置
    LCD_WR_DATA(x1 + 1);
    LCD_WR_DATA(x2 + 1);
    LCD_WR_REG(0x2b);  // 行地址设置
    LCD_WR_DATA(y1 + 2);
    LCD_WR_DATA(y2 + 2);
    LCD_WR_REG(0x2c);  // 储存器写
  } else {
    LCD_WR_REG(0x2a);  // 列地址设置
    LCD_WR_DATA(x1 + 1);
    LCD_WR_DATA(x2 + 1);
    LCD_WR_REG(0x2b);  // 行地址设置
    LCD_WR_DATA(y1 + 2);
    LCD_WR_DATA(y2 + 2);
    LCD_WR_REG(0x2c);  // 储存器写
  }
}

void LCD_Init(void) {
  LCD_RES_Clr();  // 复位
  vTaskDelay(pdMS_TO_TICKS(100));

  LCD_RES_Set();
  vTaskDelay(pdMS_TO_TICKS(100));

  //************* Start Initial Sequence **********//
  LCD_WR_REG(0x11);                // Sleep out
  vTaskDelay(pdMS_TO_TICKS(120));  // Delay 120ms

  //------------------------------------ST7735S Frame
  // Rate-----------------------------------------//
  LCD_WR_REG(0xB1);
  LCD_WR_DATA8(0x05);
  LCD_WR_DATA8(0x3C);
  LCD_WR_DATA8(0x3C);
  LCD_WR_REG(0xB2);
  LCD_WR_DATA8(0x05);
  LCD_WR_DATA8(0x3C);
  LCD_WR_DATA8(0x3C);
  LCD_WR_REG(0xB3);
  LCD_WR_DATA8(0x05);
  LCD_WR_DATA8(0x3C);
  LCD_WR_DATA8(0x3C);
  LCD_WR_DATA8(0x05);
  LCD_WR_DATA8(0x3C);
  LCD_WR_DATA8(0x3C);
  //------------------------------------End ST7735S Frame
  // Rate---------------------------------//
  LCD_WR_REG(0xB4);  // Dot inversion
  LCD_WR_DATA8(0x03);
  //------------------------------------ST7735S Power
  // Sequence---------------------------------//
  LCD_WR_REG(0xC0);
  LCD_WR_DATA8(0x28);
  LCD_WR_DATA8(0x08);
  LCD_WR_DATA8(0x04);
  LCD_WR_REG(0xC1);
  LCD_WR_DATA8(0XC0);
  LCD_WR_REG(0xC2);
  LCD_WR_DATA8(0x0D);
  LCD_WR_DATA8(0x00);
  LCD_WR_REG(0xC3);
  LCD_WR_DATA8(0x8D);
  LCD_WR_DATA8(0x2A);
  LCD_WR_REG(0xC4);
  LCD_WR_DATA8(0x8D);
  LCD_WR_DATA8(0xEE);
  //---------------------------------End ST7735S Power
  // Sequence-------------------------------------//
  LCD_WR_REG(0xC5);  // VCOM
  LCD_WR_DATA8(0x1A);
  LCD_WR_REG(0x36);  // MX, MY, RGB mode
  if (USE_HORIZONTAL == 0)
    LCD_WR_DATA8(0x00);
  else if (USE_HORIZONTAL == 1)
    LCD_WR_DATA8(0xC0);
  else if (USE_HORIZONTAL == 2)
    LCD_WR_DATA8(0x70);
  else
    LCD_WR_DATA8(0xA0);
  //------------------------------------ST7735S Gamma
  // Sequence---------------------------------//
  LCD_WR_REG(0xE0);
  LCD_WR_DATA8(0x04);
  LCD_WR_DATA8(0x22);
  LCD_WR_DATA8(0x07);
  LCD_WR_DATA8(0x0A);
  LCD_WR_DATA8(0x2E);
  LCD_WR_DATA8(0x30);
  LCD_WR_DATA8(0x25);
  LCD_WR_DATA8(0x2A);
  LCD_WR_DATA8(0x28);
  LCD_WR_DATA8(0x26);
  LCD_WR_DATA8(0x2E);
  LCD_WR_DATA8(0x3A);
  LCD_WR_DATA8(0x00);
  LCD_WR_DATA8(0x01);
  LCD_WR_DATA8(0x03);
  LCD_WR_DATA8(0x13);
  LCD_WR_REG(0xE1);
  LCD_WR_DATA8(0x04);
  LCD_WR_DATA8(0x16);
  LCD_WR_DATA8(0x06);
  LCD_WR_DATA8(0x0D);
  LCD_WR_DATA8(0x2D);
  LCD_WR_DATA8(0x26);
  LCD_WR_DATA8(0x23);
  LCD_WR_DATA8(0x27);
  LCD_WR_DATA8(0x27);
  LCD_WR_DATA8(0x25);
  LCD_WR_DATA8(0x2D);
  LCD_WR_DATA8(0x3B);
  LCD_WR_DATA8(0x00);
  LCD_WR_DATA8(0x01);
  LCD_WR_DATA8(0x04);
  LCD_WR_DATA8(0x13);
  //------------------------------------End ST7735S Gamma
  // Sequence-----------------------------//
  LCD_WR_REG(0x3A);  // 65k mode
  LCD_WR_DATA8(0x05);
  LCD_WR_REG(0x29);  // Display on

  LCD_BLK_Set();  // 打开背光
  vTaskDelay(pdMS_TO_TICKS(100));
}
