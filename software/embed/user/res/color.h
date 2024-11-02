#ifndef _COLOR_H_
#define _COLOR_H_

#include <stdint.h>

#define RGB888_2_RGB565(color) \
  (((color) >> 8) & 0xF800) + (((color) >> 5) & 0x7E0) + (((color) >> 3) & 0x1F)

#define RGB565(r, g, b) \
  (uint16_t)((((r) & 0xF8) << 8) + (((g) & 0xFC) << 3) + (((b) & 0xF8) >> 3))

extern const uint16_t RGB565_WHITE, RGB565_BLACK, RGB565_RED, RGB565_GREEN,
    RGB565_BLUE;

extern const uint16_t RAINBOW[256];

#endif