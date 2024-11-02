#include "my_lcd.h"
///
#include "color.h"
#include "font.h"
#include "lcd_init.h"
#include "util_image.h"
///
#define LOG_TAG "my lcd"
#include "elog.h"
//
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "event_groups.h"
///
#include "gd32f4xx.h"
///
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define LCD_BUF_SET(buf, x, y, color) \
  *(uint16_t *)((uint32_t)buf +       \
                ((x) > 159 ? 159      \
                 : (x) < 0 ? 0        \
                           : (x)) *   \
                    2 +               \
                ((y) > 128 ? 128      \
                 : (y) < 0 ? 0        \
                           : (y)) *   \
                    160 * 2) = ((color) << 8) + ((color) >> 8)

// const uint16_t RAINBOW[256] = {
//     RGB565(0, 255, 0),   //
//     RGB565(0, 252, 0),   //
//     RGB565(0, 249, 0),   //
//     RGB565(0, 246, 0),   //
//     RGB565(0, 243, 0),   //
//     RGB565(0, 240, 0),   //
//     RGB565(0, 237, 0),   //
//     RGB565(0, 234, 0),   //
//     RGB565(0, 231, 0),   //
//     RGB565(0, 228, 0),   //
//     RGB565(0, 225, 0),   //
//     RGB565(0, 222, 0),   //
//     RGB565(0, 219, 0),   //
//     RGB565(0, 216, 0),   //
//     RGB565(0, 213, 0),   //
//     RGB565(0, 210, 0),   //
//     RGB565(0, 207, 0),   //
//     RGB565(0, 204, 0),   //
//     RGB565(0, 201, 0),   //
//     RGB565(0, 198, 0),   //
//     RGB565(0, 195, 0),   //
//     RGB565(0, 192, 0),   //
//     RGB565(0, 189, 0),   //
//     RGB565(0, 186, 0),   //
//     RGB565(0, 183, 0),   //
//     RGB565(0, 180, 0),   //
//     RGB565(0, 177, 0),   //
//     RGB565(0, 174, 0),   //
//     RGB565(0, 171, 0),   //
//     RGB565(0, 168, 0),   //
//     RGB565(0, 165, 0),   //
//     RGB565(0, 162, 0),   //
//     RGB565(0, 159, 0),   //
//     RGB565(0, 156, 0),   //
//     RGB565(0, 153, 0),   //
//     RGB565(0, 150, 0),   //
//     RGB565(0, 147, 0),   //
//     RGB565(0, 144, 0),   //
//     RGB565(0, 141, 0),   //
//     RGB565(0, 138, 0),   //
//     RGB565(0, 135, 0),   //
//     RGB565(0, 132, 0),   //
//     RGB565(0, 129, 0),   //
//     RGB565(0, 126, 0),   //
//     RGB565(0, 123, 3),   //
//     RGB565(0, 120, 6),   //
//     RGB565(0, 117, 9),   //
//     RGB565(0, 114, 12),  //
//     RGB565(0, 111, 15),  //
//     RGB565(0, 108, 18),  //
//     RGB565(0, 105, 21),  //
//     RGB565(0, 102, 24),  //
//     RGB565(0, 99, 27),   //
//     RGB565(0, 96, 30),   //
//     RGB565(0, 93, 33),   //
//     RGB565(0, 90, 36),   //
//     RGB565(0, 87, 39),   //
//     RGB565(0, 84, 42),   //
//     RGB565(0, 81, 45),   //
//     RGB565(0, 78, 48),   //
//     RGB565(0, 75, 51),   //
//     RGB565(0, 72, 54),   //
//     RGB565(0, 69, 57),   //
//     RGB565(0, 66, 60),   //
//     RGB565(0, 63, 63),   //
//     RGB565(0, 60, 66),   //
//     RGB565(0, 57, 69),   //
//     RGB565(0, 54, 72),   //
//     RGB565(0, 51, 75),   //
//     RGB565(0, 48, 78),   //
//     RGB565(0, 45, 81),   //
//     RGB565(0, 42, 84),   //
//     RGB565(0, 39, 87),   //
//     RGB565(0, 36, 90),   //
//     RGB565(0, 33, 93),   //
//     RGB565(0, 30, 96),   //
//     RGB565(0, 27, 99),   //
//     RGB565(0, 24, 102),  //
//     RGB565(0, 21, 105),  //
//     RGB565(0, 18, 108),  //
//     RGB565(0, 15, 111),  //
//     RGB565(0, 12, 114),  //
//     RGB565(0, 9, 117),   //
//     RGB565(0, 6, 120),   //
//     RGB565(0, 3, 123),   //
//     RGB565(0, 0, 126),   //
//     RGB565(0, 0, 129),   //
//     RGB565(0, 0, 132),   //
//     RGB565(0, 0, 135),   //
//     RGB565(0, 0, 138),   //
//     RGB565(0, 0, 141),   //
//     RGB565(0, 0, 144),   //
//     RGB565(0, 0, 147),   //
//     RGB565(0, 0, 150),   //
//     RGB565(0, 0, 153),   //
//     RGB565(0, 0, 156),   //
//     RGB565(0, 0, 159),   //
//     RGB565(0, 0, 162),   //
//     RGB565(0, 0, 165),   //
//     RGB565(0, 0, 168),   //
//     RGB565(0, 0, 171),   //
//     RGB565(0, 0, 174),   //
//     RGB565(0, 0, 177),   //
//     RGB565(0, 0, 180),   //
//     RGB565(0, 0, 183),   //
//     RGB565(0, 0, 186),   //
//     RGB565(0, 0, 189),   //
//     RGB565(0, 0, 192),   //
//     RGB565(0, 0, 195),   //
//     RGB565(0, 0, 198),   //
//     RGB565(0, 0, 201),   //
//     RGB565(0, 0, 204),   //
//     RGB565(0, 0, 207),   //
//     RGB565(0, 0, 210),   //
//     RGB565(0, 0, 213),   //
//     RGB565(0, 0, 216),   //
//     RGB565(0, 0, 219),   //
//     RGB565(0, 0, 222),   //
//     RGB565(0, 0, 225),   //
//     RGB565(0, 0, 228),   //
//     RGB565(0, 0, 231),   //
//     RGB565(0, 0, 234),   //
//     RGB565(0, 0, 237),   //
//     RGB565(0, 0, 240),   //
//     RGB565(0, 0, 243),   //
//     RGB565(0, 0, 246),   //
//     RGB565(0, 0, 249),   //
//     RGB565(0, 0, 252),   //
//     RGB565(0, 0, 255),   //
//     RGB565(0, 0, 249),   //
//     RGB565(0, 0, 246),   //
//     RGB565(0, 0, 243),   //
//     RGB565(0, 0, 240),   //
//     RGB565(0, 0, 237),   //
//     RGB565(0, 0, 234),   //
//     RGB565(0, 0, 231),   //
//     RGB565(0, 0, 228),   //
//     RGB565(0, 0, 225),   //
//     RGB565(0, 0, 222),   //
//     RGB565(0, 0, 219),   //
//     RGB565(0, 0, 216),   //
//     RGB565(0, 0, 213),   //
//     RGB565(0, 0, 210),   //
//     RGB565(0, 0, 207),   //
//     RGB565(0, 0, 204),   //
//     RGB565(0, 0, 201),   //
//     RGB565(0, 0, 198),   //
//     RGB565(0, 0, 195),   //
//     RGB565(0, 0, 192),   //
//     RGB565(0, 0, 189),   //
//     RGB565(0, 0, 186),   //
//     RGB565(0, 0, 183),   //
//     RGB565(0, 0, 180),   //
//     RGB565(0, 0, 177),   //
//     RGB565(0, 0, 174),   //
//     RGB565(0, 0, 171),   //
//     RGB565(0, 0, 168),   //
//     RGB565(0, 0, 165),   //
//     RGB565(0, 0, 162),   //
//     RGB565(0, 0, 159),   //
//     RGB565(0, 0, 156),   //
//     RGB565(0, 0, 153),   //
//     RGB565(0, 0, 150),   //
//     RGB565(0, 0, 147),   //
//     RGB565(0, 0, 144),   //
//     RGB565(0, 0, 141),   //
//     RGB565(0, 0, 138),   //
//     RGB565(0, 0, 135),   //
//     RGB565(0, 0, 132),   //
//     RGB565(0, 0, 129),   //
//     RGB565(0, 0, 126),   //
//     RGB565(3, 0, 123),   //
//     RGB565(6, 0, 120),   //
//     RGB565(9, 0, 117),   //
//     RGB565(12, 0, 114),  //
//     RGB565(15, 0, 111),  //
//     RGB565(18, 0, 108),  //
//     RGB565(21, 0, 105),  //
//     RGB565(24, 0, 102),  //
//     RGB565(27, 0, 99),   //
//     RGB565(30, 0, 96),   //
//     RGB565(33, 0, 93),   //
//     RGB565(36, 0, 90),   //
//     RGB565(39, 0, 87),   //
//     RGB565(42, 0, 84),   //
//     RGB565(45, 0, 81),   //
//     RGB565(48, 0, 78),   //
//     RGB565(51, 0, 75),   //
//     RGB565(54, 0, 72),   //
//     RGB565(57, 0, 69),   //
//     RGB565(60, 0, 66),   //
//     RGB565(63, 0, 63),   //
//     RGB565(66, 0, 60),   //
//     RGB565(69, 0, 57),   //
//     RGB565(72, 0, 54),   //
//     RGB565(75, 0, 51),   //
//     RGB565(78, 0, 48),   //
//     RGB565(81, 0, 45),   //
//     RGB565(84, 0, 42),   //
//     RGB565(87, 0, 39),   //
//     RGB565(90, 0, 36),   //
//     RGB565(93, 0, 33),   //
//     RGB565(96, 0, 30),   //
//     RGB565(99, 0, 27),   //
//     RGB565(102, 0, 24),  //
//     RGB565(105, 0, 21),  //
//     RGB565(108, 0, 18),  //
//     RGB565(111, 0, 15),  //
//     RGB565(114, 0, 12),  //
//     RGB565(117, 0, 9),   //
//     RGB565(120, 0, 6),   //
//     RGB565(123, 0, 3),   //
//     RGB565(126, 0, 0),   //
//     RGB565(129, 0, 0),   //
//     RGB565(132, 0, 0),   //
//     RGB565(135, 0, 0),   //
//     RGB565(138, 0, 0),   //
//     RGB565(141, 0, 0),   //
//     RGB565(144, 0, 0),   //
//     RGB565(147, 0, 0),   //
//     RGB565(150, 0, 0),   //
//     RGB565(153, 0, 0),   //
//     RGB565(156, 0, 0),   //
//     RGB565(159, 0, 0),   //
//     RGB565(162, 0, 0),   //
//     RGB565(165, 0, 0),   //
//     RGB565(168, 0, 0),   //
//     RGB565(171, 0, 0),   //
//     RGB565(174, 0, 0),   //
//     RGB565(177, 0, 0),   //
//     RGB565(180, 0, 0),   //
//     RGB565(183, 0, 0),   //
//     RGB565(186, 0, 0),   //
//     RGB565(189, 0, 0),   //
//     RGB565(192, 0, 0),   //
//     RGB565(195, 0, 0),   //
//     RGB565(198, 0, 0),   //
//     RGB565(201, 0, 0),   //
//     RGB565(204, 0, 0),   //
//     RGB565(207, 0, 0),   //
//     RGB565(210, 0, 0),   //
//     RGB565(213, 0, 0),   //
//     RGB565(216, 0, 0),   //
//     RGB565(219, 0, 0),   //
//     RGB565(222, 0, 0),   //
//     RGB565(225, 0, 0),   //
//     RGB565(228, 0, 0),   //
//     RGB565(231, 0, 0),   //
//     RGB565(234, 0, 0),   //
//     RGB565(237, 0, 0),   //
//     RGB565(240, 0, 0),   //
//     RGB565(243, 0, 0),   //
//     RGB565(246, 0, 0),   //
//     RGB565(249, 0, 0),   //
//     RGB565(252, 0, 0),   //
//     RGB565(255, 0, 0),   //
// };

uint16_t lcd_buf[160 * 128] = {0};

static SemaphoreHandle_t buf_mutex_handle __attribute__((section(".DTCM")));
static StaticSemaphore_t buf_mutex __attribute__((section(".DTCM")));

static TaskHandle_t flush_task_handle __attribute__((section(".DTCM")));
static StaticTask_t flush_task __attribute__((section(".DTCM")));
static uint8_t flush_task_stack[1024] __attribute__((section(".DTCM")));

static SemaphoreHandle_t lcd_flash_compl_sem_handle
    __attribute__((section(".DTCM")));
static StaticSemaphore_t lcd_flash_compl_sem __attribute__((section(".DTCM")));

static void lcd_buf_flush() {
  dma_single_data_parameter_struct init_struct_s = {
      .periph_addr         = (uint32_t)&SPI_DATA(SPI1),
      .periph_inc          = DMA_PERIPH_INCREASE_DISABLE,
      .memory0_addr        = (uint32_t)lcd_buf,
      .memory_inc          = DMA_MEMORY_INCREASE_ENABLE,
      .periph_memory_width = DMA_PERIPH_WIDTH_8BIT,
      .circular_mode       = DMA_CIRCULAR_MODE_DISABLE,
      .direction           = DMA_MEMORY_TO_PERIPH,
      .number              = sizeof(lcd_buf),
      .priority            = DMA_PRIORITY_HIGH,
  };

  while (1) {
    // 等待刷新请求
    ulTaskNotifyTake(0xFFFFFFFF, portMAX_DELAY);

    lcd_buf_lock();

    LCD_Address_Set(0, 0, LCD_W - 1, LCD_H - 1);

    LCD_CS_Clr();

    dma_deinit(DMA0, DMA_CH4);
    dma_single_data_mode_init(DMA0, DMA_CH4, &init_struct_s);
    dma_interrupt_flag_clear(DMA0, DMA_CH4, DMA_INT_FLAG_FTF);
    dma_interrupt_enable(DMA0, DMA_CH4, DMA_CHXCTL_FTFIE);
    dma_channel_enable(DMA0, DMA_CH4);

    xSemaphoreTake(lcd_flash_compl_sem_handle, portMAX_DELAY);

    lcd_buf_unlock();

    LCD_CS_Set();
  }
}

void lcd_refreash_request() { xTaskNotifyGive(flush_task_handle); }

void inline lcd_buf_lock() { xSemaphoreTake(buf_mutex_handle, portMAX_DELAY); }

void inline lcd_buf_unlock() { xSemaphoreGive(buf_mutex_handle); }

void lcd_handler_init() {
  buf_mutex_handle = xSemaphoreCreateMutexStatic(&buf_mutex);
  configASSERT(buf_mutex_handle);

  lcd_flash_compl_sem_handle =
      xSemaphoreCreateBinaryStatic(&lcd_flash_compl_sem);
  configASSERT(lcd_flash_compl_sem_handle);

  configASSERT(!(sizeof(flush_task_stack) % 4));
  flush_task_handle =
      xTaskCreateStatic(lcd_buf_flush, "lcdFlush", sizeof(flush_task_stack) / 4,
                        NULL, 8, (StackType_t *)flush_task_stack, &flush_task);
  configASSERT(flush_task_handle);

  lcd_fill(0, 0, LCD_W, LCD_H, 0);

  lcd_refreash_request();
}

static void lcd_fill_unmutex(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                             uint16_t color) {
  for (size_t i = x0; i <= x1; i++)
    for (size_t j = y0; j <= y1; j++) LCD_BUF_SET(lcd_buf, i, j, color);
}

void lcd_fill(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
              uint16_t color) {
  lcd_buf_lock();

  lcd_fill_unmutex(x0, y0, x1, y1, color);

  lcd_buf_unlock();
}

void lcd_draw_point(uint16_t x, uint16_t y, uint16_t color) {
  lcd_buf_lock();

  LCD_BUF_SET(lcd_buf, x, y, color);

  lcd_buf_unlock();
}

void lcd_draw_line_unmutex(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                           uint16_t color) {
  uint16_t t;
  int xerr = 0, yerr = 0, delta_x, delta_y, distance;
  int incx, incy, uRow, uCol;
  delta_x = x2 - x1;  // 计算坐标增量
  delta_y = y2 - y1;
  uRow    = x1;  // 画线起点坐标
  uCol    = y1;
  if (delta_x > 0)
    incx = 1;  // 设置单步方向
  else if (delta_x == 0)
    incx = 0;  // 垂直线
  else {
    incx    = -1;
    delta_x = -delta_x;
  }
  if (delta_y > 0)
    incy = 1;
  else if (delta_y == 0)
    incy = 0;  // 水平线
  else {
    incy    = -1;
    delta_y = -delta_y;
  }
  if (delta_x > delta_y)
    distance = delta_x;  // 选取基本增量坐标轴
  else
    distance = delta_y;
  for (t = 0; t < distance + 1; t++) {
    LCD_BUF_SET(lcd_buf, uRow, uCol, color);  // 画点
    xerr += delta_x;
    yerr += delta_y;
    if (xerr > distance) {
      xerr -= distance;
      uRow += incx;
    }
    if (yerr > distance) {
      yerr -= distance;
      uCol += incy;
    }
  }
}

void lcd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                   uint16_t color) {
  lcd_buf_lock();

  lcd_draw_line_unmutex(x1, y1, x2, y2, color);

  lcd_buf_unlock();
}

void lcd_draw_multi_line(uint16_t *x, uint16_t *y, uint16_t point_num,
                         uint16_t color) {
  if (point_num < 2) return;

  lcd_buf_lock();

  for (size_t i = 0; i < point_num - 1; i++)
    lcd_draw_line_unmutex(
        *(uint16_t *)((uint32_t)x + i * sizeof(uint16_t)),
        *(uint16_t *)((uint32_t)y + i * sizeof(uint16_t)),
        *(uint16_t *)((uint32_t)x + (i + 1) * sizeof(uint16_t)),
        *(uint16_t *)((uint32_t)y + (i + 1) * sizeof(uint16_t)), color);

  lcd_buf_unlock();
}

void lcd_draw_rectangle_unmux(uint16_t x1, uint16_t y1, uint16_t x2,
                              uint16_t y2, uint16_t color) {
  if (x1 >= 160 || x2 >= 160 || y1 >= 128 || y2 >= 128) return;
  lcd_draw_line_unmutex(x1, y1, x2, y1, color);
  lcd_draw_line_unmutex(x1, y1, x1, y2, color);
  lcd_draw_line_unmutex(x1, y2, x2, y2, color);
  lcd_draw_line_unmutex(x2, y1, x2, y2, color);
}

void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                        uint16_t color) {
  lcd_buf_lock();

  lcd_draw_rectangle_unmux(x1, y1, x2, y2, color);

  lcd_buf_unlock();
}

void lcd_draw_circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color) {
  lcd_buf_lock();

  uint16_t a, b;
  a = 0;
  b = r;
  while (a <= b) {
    LCD_BUF_SET(lcd_buf, x0 - b, y0 - a, color);
    LCD_BUF_SET(lcd_buf, x0 + b, y0 - a, color);
    LCD_BUF_SET(lcd_buf, x0 - a, y0 + b, color);
    LCD_BUF_SET(lcd_buf, x0 - a, y0 - b, color);
    LCD_BUF_SET(lcd_buf, x0 + b, y0 + a, color);
    LCD_BUF_SET(lcd_buf, x0 + a, y0 - b, color);
    LCD_BUF_SET(lcd_buf, x0 + a, y0 + b, color);
    LCD_BUF_SET(lcd_buf, x0 - b, y0 + a, color);
    a++;
    if ((a * a + b * b) > (r * r))  // 判断要画的点是否过远
      b--;
  }

  lcd_buf_unlock();
}

static void my_lcd_show_hanz12(uint16_t x, uint16_t y, uint8_t *s, uint16_t fc,
                               uint16_t bc) {
  const static uint8_t FONT_SIZE_PIXEL = 12,
                       ROW_BYTE =
                           FONT_SIZE_PIXEL / 8 + (FONT_SIZE_PIXEL % 8 ? 1 : 0);

  if (fc != bc) lcd_fill_unmutex(x, y, x + 16, y + 16, bc);

  uint32_t font_idx = 0;

  // 查找字库
  for (font_idx = 0; font_idx < FONT12_SIZE; font_idx++) {
    if (FONT12_DATA[font_idx].txt[0] == s[0] &&
        FONT12_DATA[font_idx].txt[1] == s[1] &&
        FONT12_DATA[font_idx].txt[2] == s[2]) {
      // 找到对应字符
      for (size_t i = 0; i < FONT_SIZE_PIXEL; i++)
        for (size_t j = 0; j < FONT_SIZE_PIXEL; j++)
          if ((FONT12_DATA[font_idx].dat[i * ROW_BYTE + (j >> 3)] >>
               ((ROW_BYTE * 8 - 1 - j) & (8 - 1))) &
              0x1)
            LCD_BUF_SET(lcd_buf, x + j, y + i, fc);
      return;
    }
  }

  // 未找到对应字符，使用空字符填充
  for (size_t i = 0; i < FONT_SIZE_PIXEL; i++)
    for (size_t j = 0; j < FONT_SIZE_PIXEL; j++)
      if ((FONT12_NULL.dat[i * ROW_BYTE + (j >> 3)] >>
           ((ROW_BYTE * 8 - 1 - j) & (8 - 1))) &
          0x1)
        LCD_BUF_SET(lcd_buf, x + j, y + i, fc);
}

static void my_lcd_show_hanz14(uint16_t x, uint16_t y, uint8_t *s, uint16_t fc,
                               uint16_t bc) {
  const static uint8_t FONT_SIZE_PIXEL = 14,
                       ROW_BYTE =
                           FONT_SIZE_PIXEL / 8 + (FONT_SIZE_PIXEL % 8 ? 1 : 0);

  if (fc != bc) lcd_fill_unmutex(x, y, x + 16, y + 16, bc);

  uint32_t font_idx = 0;

  // 查找字库
  for (font_idx = 0; font_idx < FONT12_SIZE; font_idx++) {
    if (FONT14_DATA[font_idx].txt[0] == s[0] &&
        FONT14_DATA[font_idx].txt[1] == s[1] &&
        FONT14_DATA[font_idx].txt[2] == s[2]) {
      // 找到对应字符
      for (size_t i = 0; i < FONT_SIZE_PIXEL; i++)
        for (size_t j = 0; j < FONT_SIZE_PIXEL; j++)
          if ((FONT14_DATA[font_idx].dat[i * ROW_BYTE + (j >> 3)] >>
               ((ROW_BYTE * 8 - 1 - j) & (8 - 1))) &
              0x1)
            LCD_BUF_SET(lcd_buf, x + j, y + i, fc);
      return;
    }
  }

  // 未找到对应字符，使用空字符填充
  for (size_t i = 0; i < FONT_SIZE_PIXEL; i++)
    for (size_t j = 0; j < FONT_SIZE_PIXEL; j++)
      if ((FONT14_NULL.dat[i * ROW_BYTE + (j >> 3)] >>
           ((ROW_BYTE * 8 - 1 - j) & (8 - 1))) &
          0x1)
        LCD_BUF_SET(lcd_buf, x + j, y + i, fc);
}

static void my_lcd_show_hanz16(uint16_t x, uint16_t y, uint8_t *s, uint16_t fc,
                               uint16_t bc) {
  const static uint8_t FONT_SIZE_PIXEL = 16,
                       ROW_BYTE =
                           FONT_SIZE_PIXEL / 8 + (FONT_SIZE_PIXEL % 8 ? 1 : 0);

  if (fc != bc) lcd_fill_unmutex(x, y, x + 16, y + 16, bc);

  uint32_t font_idx = 0;

  // 查找字库
  for (font_idx = 0; font_idx < FONT16_SIZE; font_idx++) {
    if (FONT16_DATA[font_idx].txt[0] == s[0] &&
        FONT16_DATA[font_idx].txt[1] == s[1] &&
        FONT16_DATA[font_idx].txt[2] == s[2]) {
      // 找到对应字符
      for (size_t i = 0; i < FONT_SIZE_PIXEL; i++)
        for (size_t j = 0; j < FONT_SIZE_PIXEL; j++)
          if ((FONT16_DATA[font_idx].dat[i * ROW_BYTE + (j >> 3)] >>
               ((ROW_BYTE * 8 - 1 - j) & (8 - 1))) &
              0x1)
            LCD_BUF_SET(lcd_buf, x + j, y + i, fc);
      return;
    }
  }

  // 未找到对应字符，使用空字符填充
  for (size_t i = 0; i < FONT_SIZE_PIXEL; i++)
    for (size_t j = 0; j < FONT_SIZE_PIXEL; j++)
      if ((FONT16_NULL.dat[i * ROW_BYTE + ((j) >> 3)] >>
           ((ROW_BYTE * 8 - 1 - j) & (8 - 1))) &
          0x1)
        LCD_BUF_SET(lcd_buf, x + j, y + i, fc);
}

void lcd_show_font(uint16_t x, uint16_t y, uint8_t *s, uint16_t fc, uint16_t bc,
                   FontSize_t fs) {
  lcd_buf_lock();

  while (*s != 0) {
    switch (fs) {
      case FontSize12: {
        my_lcd_show_hanz12(x, y, s, fc, bc);
        x += 12;
        break;
      }
      case FontSize14: {
        my_lcd_show_hanz14(x, y, s, fc, bc);
        x += 14;
        break;
      }
      case FontSize16: {
        my_lcd_show_hanz16(x, y, s, fc, bc);
        x += 16;
        break;
      }
      default:
        break;
    }
    s += 3;
  }

  lcd_buf_unlock();
}

static void lcd_show_char(uint16_t x, uint16_t y, uint8_t num, uint16_t fc,
                          uint16_t bc, uint8_t sizey) {
  uint8_t temp, sizex, t;
  uint16_t i, TypefaceNum;  // 一个字符所占字节大小
  uint16_t x0 = x;
  sizex       = sizey / 2;
  TypefaceNum = (sizex / 8 + ((sizex % 8) ? 1 : 0)) * sizey;
  num         = num - ' ';

  if (fc != bc) lcd_fill_unmutex(x, y, x + sizex - 1, y + sizey - 1, bc);

  for (i = 0; i < TypefaceNum; i++) {
    if (sizey == 12)
      temp = ascii_1206[num][i];  // 调用6x12字体
    else if (sizey == 16)
      temp = ascii_1608[num][i];  // 调用8x16字体
    else if (sizey == 24)
      temp = ascii_2412[num][i];  // 调用12x24字体
    else if (sizey == 32)
      temp = ascii_3216[num][i];  // 调用16x32字体
    else
      return;
    for (t = 0; t < 8; t++) {
      if (temp & (0x01 << t)) LCD_BUF_SET(lcd_buf, x, y, fc);
      x++;
      if ((x - x0) == sizex) {
        x = x0;
        y++;
        break;
      }
    }
  }
}

void lcd_show_str(uint16_t x, uint16_t y, const uint8_t *p, uint16_t fc,
                  uint16_t bc, uint8_t sizey) {
  lcd_buf_lock();

  while (*p != '\0') {
    lcd_show_char(x, y, *p, fc, bc, sizey);
    x += sizey / 2;
    p++;
  }

  lcd_buf_unlock();
}

void lcd_show_num(uint16_t x, uint16_t y, uint16_t num, uint8_t len,
                  uint16_t fc, uint16_t bc, uint8_t sizey) {
  lcd_buf_lock();

  uint8_t t, temp;
  uint8_t enshow = 0;
  uint8_t sizex  = sizey / 2;
  for (t = 0; t < len; t++) {
    temp = (uint16_t)(num / powf(10, len - t - 1)) % 10;
    if (enshow == 0 && t < (len - 1)) {
      if (temp == 0) {
        lcd_show_char(x + t * sizex, y, ' ', fc, bc, sizey);
        continue;
      } else
        enshow = 1;
    }
    lcd_show_char(x + t * sizex, y, temp + 48, fc, bc, sizey);
  }

  lcd_buf_unlock();
}

void lcd_show_float(uint16_t x, uint16_t y, float num, uint8_t len, uint16_t fc,
                    uint16_t bc, uint8_t sizey) {
  lcd_buf_lock();

  uint8_t t, temp, sizex;
  uint16_t num1;
  sizex = sizey / 2;
  num1  = num * 100;
  for (t = 0; t < len; t++) {
    temp = (uint16_t)(num1 / pow(10, len - t - 1)) % 10;
    if (t == (len - 1)) {
      lcd_show_char(x + (len - 2) * sizex, y, '.', fc, bc, sizey);
      t++;
      len += 1;
    }
    lcd_show_char(x + t * sizex, y, temp + 48, fc, bc, sizey);
  }

  lcd_buf_unlock();
}

void lcd_show_rgb565_pic(uint16_t x, uint16_t y, uint16_t height,
                         uint16_t width, const uint16_t *pic) {
  configASSERT(x + height <= LCD_H);
  configASSERT(y + width <= LCD_W);

  lcd_buf_lock();

  for (size_t i = 0; i < height; i++)
    for (size_t j = 0; j < width; j++) {
      LCD_BUF_SET(lcd_buf, x + j, y + i, *pic);
      pic++;
    }

  lcd_buf_unlock();
}

void lcd_show_battery(const uint16_t x, const uint16_t y,
                      const BatteryLevel_t bat_lvl) {
  // 电池块宽高
  const uint8_t BAT_W = 3U, BAT_H = 6U;

  // 电池图标内部宽高
  const uint8_t BAT_ICON_INNER_W = BAT_W * 3U + 1U * 4U,
                BAT_ICON_INNER_H = BAT_H + 2U;

  // 电池图标外部宽高
  const uint8_t BAT_ICON_OUTER_W = BAT_ICON_INNER_W + 2U * 2U,
                BAT_ICON_OUTER_H = BAT_ICON_INNER_H + 2U * 2U;

  lcd_buf_lock();

  lcd_fill_unmutex(x, y, x + BAT_ICON_OUTER_W - 1, y + BAT_ICON_OUTER_H - 1,
                   RGB565_WHITE);
  lcd_fill_unmutex(x + 2, y + 2, x + 2 + BAT_ICON_INNER_W - 1,
                   y + 2 + BAT_ICON_INNER_H - 1, RGB565_BLACK);
  lcd_fill_unmutex(x + BAT_ICON_OUTER_W, y + 4, x + BAT_ICON_OUTER_W,
                   y + 4 + 4 - 1, RGB565_WHITE);

  switch (bat_lvl) {
    case BatteryLevelCharge:
      lcd_fill_unmutex(x + 7, y + 3, x + 7 + 1, y + 3 + BAT_H - 1,
                       RGB565_WHITE);
      lcd_fill_unmutex(x + 5, y + 5, x + 10, y + 5 + 1, RGB565_WHITE);
      break;
    case BatteryLevelHigh:
      lcd_fill_unmutex(x + 3 + (BAT_W + 1) * 2, y + 3,
                       x + 3 + (BAT_W + 1) * 2 - 1 + BAT_W, y + 3 + BAT_H - 1,
                       RGB565_WHITE);
    case BatteryLevelMid:
      lcd_fill_unmutex(x + 3 + BAT_W + 1, y + 3, x + 3 + BAT_W + 1 + BAT_W - 1,
                       y + 3 + BAT_H - 1, RGB565_WHITE);
    case BatteryLevelLow:
      lcd_fill_unmutex(x + 3, y + 3, x + 3 + BAT_W - 1, y + 3 + BAT_H - 1,
                       RGB565_WHITE);
      break;
    case BatteryLevelUrgent:
      break;
    default:
      break;
  }

  lcd_buf_unlock();
}

void lcd_show_rainbow(const uint16_t x, const uint16_t y) {
  const uint8_t RAINBOW_W = 100;
  const uint8_t RAINBOW_H = 10;

  lcd_buf_lock();

  lcd_fill_unmutex(x, y, x + RAINBOW_W, y + RAINBOW_H, RGB565_WHITE);

  for (size_t i = 0; i < RAINBOW_W - 4; i++) {
    lcd_fill_unmutex(x + i + 2, y + 2, x + i + 2, y + RAINBOW_H - 2,
                     RAINBOW[(uint32_t)roundf(i * 255 / (RAINBOW_W - 4))]);
  }

  lcd_buf_unlock();
}

void lcd_show_sight_unmux(const uint16_t x, const uint16_t y) {
  if (x < 4 || x > 156 || y < 4 || y > 123) return;

  lcd_draw_rectangle_unmux(x - 3, y - 3, x + 3, y + 3, RGB565_WHITE);
  lcd_draw_rectangle_unmux(x - 4, y - 4, x + 4, y + 4, RGB565_WHITE);
}

void lcd_show_sight(const uint16_t x, const uint16_t y) {
  lcd_buf_lock();

  lcd_show_sight_unmux(x, y);

  lcd_buf_unlock();
}

void DMA0_Channel4_IRQHandler() {
  if (dma_interrupt_flag_get(DMA0, DMA_CH4, DMA_INT_FLAG_FTF)) {
    xSemaphoreGiveFromISR(lcd_flash_compl_sem_handle, NULL);
    dma_interrupt_flag_clear(DMA0, DMA_CH4, DMA_INT_FLAG_FTF);
  }
}
