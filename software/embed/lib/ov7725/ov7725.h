#ifndef _OV7725_H_
#define _OV7725_H_

#include "gd32f4xx.h"

#include <stdint.h>

extern const uint32_t OV7725_RST_GPIO_PORT, OV7725_RST_GPIO_PIN;

/* OV7725 SCCB通讯地址 */
#define OV7725_SCCB_ADDR 0x21

/* OV7725模块在不同输出模式下的最大输出分辨率 */
#define OV7725_VGA_WIDTH_MAX   640
#define OV7725_VGA_HEIGHT_MAX  480
#define OV7725_QVGA_WIDTH_MAX  320
#define OV7725_QVGA_HEIGHT_MAX 240

/* OV7725模块灯光模式枚举 */
typedef enum {
  OV7725LightModeAuto = 0x00,
  OV7725_LIGHT_MODE_SUNNY,
  OV7725_LIGHT_MODE_CLOUDY,
  OV7725_LIGHT_MODE_OFFICE,
  OV7725_LIGHT_MODE_HOME,
  OV7725_LIGHT_MODE_NIGHT,
} OV7725LightMode_t;

/* OV7725模块色彩饱和度枚举 */
typedef enum {
  OV7725_COLOR_SATURATION_0 = 0x00,  // +4
  OV7725_COLOR_SATURATION_1,         // +3
  OV7725_COLOR_SATURATION_2,         // +2
  OV7725_COLOR_SATURATION_3,         // +1
  OV7725_COLOR_SATURATION_4,         // 0
  OV7725_COLOR_SATURATION_5,         // -1
  OV7725_COLOR_SATURATION_6,         // -2
  OV7725_COLOR_SATURATION_7,         // -3
  OV7725_COLOR_SATURATION_8,         // -4
} OV7725ColorSaturation_t;

/* OV7725模块亮度枚举 */
typedef enum {
  OV7725_BRIGHTNESS_0 = 0x00,  // +4
  OV7725_BRIGHTNESS_1,         // +3
  OV7725_BRIGHTNESS_2,         // +2
  OV7725_BRIGHTNESS_3,         // +1
  OV7725_BRIGHTNESS_4,         // 0
  OV7725_BRIGHTNESS_5,         // -1
  OV7725_BRIGHTNESS_6,         // -2
  OV7725_BRIGHTNESS_7,         // -3
  OV7725_BRIGHTNESS_8,         // -4
} OV7725Brightness_t;

/* OV7725模块对比度枚举 */
typedef enum {
  OV7725_CONTRAST_0 = 0x00,  // +4
  OV7725_CONTRAST_1,         // +3
  OV7725_CONTRAST_2,         // +2
  OV7725_CONTRAST_3,         // +1
  OV7725_CONTRAST_4,         // 0
  OV7725_CONTRAST_5,         // -1
  OV7725_CONTRAST_6,         // -2
  OV7725_CONTRAST_7,         // -3
  OV7725_CONTRAST_8,         // -4
} OV7725Contrast_t;

/* OV7725模块特殊效果枚举 */
typedef enum {
  OV7725_SPECIAL_EFFECT_NORMAL = 0x00,
  OV7725_SPECIAL_EFFECT_BW,
  OV7725_SPECIAL_EFFECT_BLUISH,
  OV7725_SPECIAL_EFFECT_SEPIA,
  OV7725_SPECIAL_EFFECT_REDISH,
  OV7725_SPECIAL_EFFECT_GREENISH,
  OV7725_SPECIAL_EFFECT_NEGATIVE,

} OV7725SpecialEffect_t;

/* OV7725模块输出模式枚举 */
typedef enum {
  // VGA
  OV7725_OUTPUT_MODE_VGA = 0x00,

  // QVGA
  OV7725_OUTPUT_MODE_QVGA,
} OV7725OutputMode_t;

/* OV7725获取帧数据方式枚举 */
typedef enum {

  // 图像数据以字节方式写入目的地址，目的地址固定不变
  OV7725_GET_TYPE_DTS_8B_NOINC = 0x00,

  // 图像数据以字节方式写入目的地址，目的地址自动增加
  OV7725_GET_TYPE_DTS_8B_INC,

  // 图像数据以半字方式写入目的地址，目的地址固定不变
  OV7725_GET_TYPE_DTS_16B_NOINC,

  // 图像数据以半字方式写入目的地址，目的地址自动增加
  OV7725_GET_TYPE_DTS_16B_INC,

  // 图像数据以字方式写入目的地址，目的地址固定不变
  OV7725_GET_TYPE_DTS_32B_NOINC,

  // 图像数据以字方式写入目的地址，目的地址自动增加
  OV7725_GET_TYPE_DTS_32B_INC,

} OV7725GetType_t;

/* 错误代码 */
#define OV7725_EOK    0 /* 没有错误 */
#define OV7725_ERROR  1 /* 错误 */
#define OV7725_EINVAL 2 /* 非法参数 */
#define OV7725_ENOMEM 3 /* 内存不足 */
#define OV7725_EEMPTY 4 /* 资源为空 */

/**
 * @brief 初始化ov7725模块
 *
 * @return uint8_t
 */
uint8_t ov7725_init();

/**
 * @brief 设置ov7725模块灯光模式
 *
 * @param mode
 * @return uint8_t
 */
uint8_t ov7725_set_light_mode(OV7725LightMode_t mode);

/**
 * @brief 设置ov7725模块色彩饱和度
 *
 * @param saturation
 * @return uint8_t
 */
uint8_t ov7725_set_color_saturation(OV7725ColorSaturation_t saturation);

/**
 * @brief 设置ov7725模块亮度
 *
 * @param brightness
 * @return uint8_t
 */
uint8_t ov7725_set_brightness(OV7725Brightness_t brightness);

/**
 * @brief 设置ov7725模块对比度
 *
 * @param contrast
 * @return uint8_t
 */
uint8_t ov7725_set_contrast(OV7725Contrast_t contrast);

/**
 * @brief 设置ov7725模块特殊效果
 *
 * @param effect
 * @return uint8_t
 */
uint8_t ov7725_set_special_effect(OV7725SpecialEffect_t effect);

/**
 * @brief 设置ov7725模块输出模式
 *
 * @param width
 * @param height
 * @param mode
 * @return uint8_t
 */
uint8_t ov7725_set_output(uint16_t width, uint16_t height,
                          OV7725OutputMode_t mode);

/**
 * @brief 设置ov7725模块时钟预分频系数
 *
 * @param clk_rc
 * @return uint8_t
 */
uint8_t ov7725_set_output_speed(uint8_t clk_rc);

/**
 * @brief 开启ov7725模块彩条测试
 *
 */
void ov7725_colorbar_enable(void);

/**
 * @brief 关闭ov7725模块彩条测试
 *
 */
void ov7725_colorbar_disable(void);

/**
 * @brief 获取ov7725模块输出图像宽度
 *
 * @return uint16_t
 */
uint16_t ov7725_get_output_width(void);

/**
 * @brief 获取ov725模块输出图像高度
 *
 * @return uint16_t
 */
uint16_t ov7725_get_output_height(void);

#endif
