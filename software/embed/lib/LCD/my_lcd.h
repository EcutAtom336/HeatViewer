#ifndef _MY_LCD_H_
#define _MY_LCD_H_

#include "power_manage.h"
///
#include <stdint.h>
#include <math.h>

extern uint16_t lcd_buf[160 * 128];

typedef enum {
  FontSize12,
  FontSize14,
  FontSize16,
} FontSize_t;

void lcd_handler_init();

void lcd_refreash_request();

void lcd_buf_lock();

void lcd_buf_unlock();

void lcd_fill(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
              uint16_t color);

// 图形相关
void lcd_draw_point(uint16_t x, uint16_t y, uint16_t color);

void lcd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                   uint16_t color);

void lcd_draw_multi_line(uint16_t *x, uint16_t *y, uint16_t num,
                         uint16_t color);

void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                        uint16_t color);

void lcd_draw_circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color);

// 字符相关
void lcd_show_font(uint16_t x, uint16_t y, uint8_t *s, uint16_t fc, uint16_t bc,
                   FontSize_t fs);

void lcd_show_str(uint16_t x, uint16_t y, const uint8_t *p, uint16_t fc,
                  uint16_t bc, uint8_t sizey);

void lcd_show_num(uint16_t x, uint16_t y, uint16_t num, uint8_t len,
                  uint16_t fc, uint16_t bc, uint8_t sizey);

void lcd_show_float(uint16_t x, uint16_t y, float num, uint8_t len, uint16_t fc,
                    uint16_t bc, uint8_t sizey);

void lcd_show_rgb565_pic(uint16_t x, uint16_t y, uint16_t height,
                         uint16_t width, const uint16_t *pic);

// expand
void lcd_show_battery(const uint16_t x, const uint16_t y,
                      const BatteryLevel_t bat_lvl);

void lcd_show_rainbow(const uint16_t x, const uint16_t y);

void lcd_show_sight_unmux(const uint16_t x, const uint16_t y);

void lcd_show_sight(const uint16_t x, const uint16_t y);

#endif