#include "ov7725.h"
#include "ov7725_sccb.h"
#include "ov7725_cfg.h"
///
#define LOG_TAG "ov7725"
#include "elog.h"
///
#include "FreeRTOS.h"
#include "task.h"
///
#include "gd32f4xx.h"

const uint32_t OV7725_RST_GPIO_PORT = GPIOD, OV7725_RST_GPIO_PIN = GPIO_PIN_7;
const uint16_t OV7725_MID = 0x7FA2, OV7725_PID = 0x7721;

/* OV7725模块数据结构体 */
static struct {
  struct {
    uint16_t width;
    uint16_t height;
  } output;
} g_ov7725_sta = {0};

/**
 * @brief       OV7725模块写寄存器
 * @param       reg: 寄存器地址
 *              dat: 待写入的值
 * @retval      无
 */
static void ov7725_write_reg(uint8_t reg, uint8_t dat) {
  ov7725_sccb_3_phase_write(OV7725_SCCB_ADDR, reg, dat);
}

/**
 * @brief       OV7725模块读寄存器
 * @param       reg: 寄存器的地址
 * @retval      读取到的寄存器值
 */
static uint8_t ov7725_read_reg(uint8_t reg) {
  uint8_t dat = 0;

  ov7725_sccb_2_phase_write(OV7725_SCCB_ADDR, reg);
  ov7725_sccb_2_phase_read(OV7725_SCCB_ADDR, &dat);

  return dat;
}

/**
 * @brief       OV7725模块硬件复位
 * @param       无
 * @retval      无
 */
static void ov7725_hw_reset(void) {
  gpio_bit_reset(OV7725_RST_GPIO_PORT, OV7725_RST_GPIO_PIN);
  vTaskDelay(pdMS_TO_TICKS(10));
  gpio_bit_set(OV7725_RST_GPIO_PORT, OV7725_RST_GPIO_PIN);
  vTaskDelay(pdMS_TO_TICKS(10));
}

/**
 * @brief       OV7725模块软件复位
 * @param       无
 * @retval      无
 */
static void ov7725_sw_reset(void) {
  ov7725_write_reg(OV7725_REG_COM7, 0x80);
  vTaskDelay(pdMS_TO_TICKS(50));
}

/**
 * @brief       获取OV7725模块制造商ID
 * @param       无
 * @retval      制造商ID
 */
static uint16_t ov7725_get_mid(void) {
  uint16_t mid;

  mid = ov7725_read_reg(OV7725_REG_MIDH) << 8;
  mid |= ov7725_read_reg(OV7725_REG_MIDL);

  return mid;
}

/**
 * @brief       获取OV7725模块产品ID
 * @param       无
 * @retval      产品ID
 */
static uint16_t ov7725_get_pid(void) {
  uint16_t pid;

  pid = ov7725_read_reg(OV7725_REG_PID) << 8;
  pid |= ov7725_read_reg(OV7725_REG_VER);

  return pid;
}

/**
 * @brief       初始化OV7725寄存器配置
 * @param       无
 * @retval      无
 */
static void ov7725_init_reg(void) {
  uint32_t cfg_index;

  for (cfg_index = 0;
       cfg_index < (sizeof(ov7725_init_cfg) / sizeof(ov7725_init_cfg[0]));
       cfg_index++) {
    ov7725_write_reg(ov7725_init_cfg[cfg_index][0],
                     ov7725_init_cfg[cfg_index][1]);
    switch (ov7725_init_cfg[cfg_index][0]) {
      case OV7725_REG_HSIZE: {
        g_ov7725_sta.output.width = ov7725_init_cfg[cfg_index][1] << 2;
        break;
      }
      case OV7725_REG_VSIZE: {
        g_ov7725_sta.output.height = ov7725_init_cfg[cfg_index][1] << 1;
        break;
      }
      default: {
        break;
      }
    }
  }
}

/**
 * @brief       初始化OV7725模块
 * @param       无
 * @retval      OV7725_EOK   : OV7725模块初始化成功
 *              OV7725_ERROR : 通讯出错，OV7725模块初始化失败
 */
uint8_t ov7725_init(void) {
  uint16_t mid;
  uint16_t pid;

  ov7725_hw_reset(); /* OV7725模块硬件复位 */
  ov7725_sw_reset(); /* OV7725模块软件复位 */

  mid = ov7725_get_mid(); /* 获取制造商ID */
  if (mid != OV7725_MID) {
    log_e("ov7725 mid error, acquire mid: 0x%.2x%.2x", mid >> 8, mid & 0xFF);
    return OV7725_ERROR;
  }
  log_i("ov7725 mid: 0x%x%x", mid >> 8, mid & 0xFF);

  pid = ov7725_get_pid(); /* 获取产品ID */
  if (pid != OV7725_PID) {
    log_e("ov7725 pid error, acquire pid: 0x%.2x%.2x", pid >> 8, pid & 0xFF);
    return OV7725_ERROR;
  }
  log_i("ov7725 pid: 0x%x%x", pid >> 8, pid & 0xFF);

  ov7725_init_reg();

  return OV7725_EOK;
}

/**
 * @brief       设置OV7725模块灯光模式
 * @param       mode: OV7725_LIGHT_MOED_AUTO  : Auto
 *                    OV7725_LIGHT_MOED_SUNNY : Sunny
 *                    OV7725_LIGHT_MOED_CLOUDY: Cloudy
 *                    OV7725_LIGHT_MOED_OFFICE: Office
 *                    OV7725_LIGHT_MOED_HOME  : Home
 *                    OV7725_LIGHT_MOED_NIGHT : Night
 * @retval      OV7725_EOK   : 设置OV7725模块灯光模式成功
 *              OV7725_EINVAL: 传入参数错误
 */
uint8_t ov7725_set_light_mode(OV7725LightMode_t mode) {
  switch (mode) {
    case OV7725LightModeAuto: {
      ov7725_write_reg(OV7725_REG_COM8, 0xFF);
      ov7725_write_reg(OV7725_REG_COM5, 0x65);
      ov7725_write_reg(OV7725_REG_ADVFL, 0x00);
      ov7725_write_reg(OV7725_REG_ADVFH, 0x00);
      break;
    }
    case OV7725_LIGHT_MODE_SUNNY: {
      ov7725_write_reg(OV7725_REG_COM8, 0xFD);
      ov7725_write_reg(OV7725_REG_BLUE, 0x5A);
      ov7725_write_reg(OV7725_REG_RED, 0x5C);
      ov7725_write_reg(OV7725_REG_COM5, 0x65);
      ov7725_write_reg(OV7725_REG_ADVFL, 0x00);
      ov7725_write_reg(OV7725_REG_ADVFH, 0x00);
      break;
    }
    case OV7725_LIGHT_MODE_CLOUDY: {
      ov7725_write_reg(OV7725_REG_COM8, 0xFD);
      ov7725_write_reg(OV7725_REG_BLUE, 0x58);
      ov7725_write_reg(OV7725_REG_RED, 0x60);
      ov7725_write_reg(OV7725_REG_COM5, 0x65);
      ov7725_write_reg(OV7725_REG_ADVFL, 0x00);
      ov7725_write_reg(OV7725_REG_ADVFH, 0x00);
      break;
    }
    case OV7725_LIGHT_MODE_OFFICE: {
      ov7725_write_reg(OV7725_REG_COM8, 0xFD);
      ov7725_write_reg(OV7725_REG_BLUE, 0x84);
      ov7725_write_reg(OV7725_REG_RED, 0x4C);
      ov7725_write_reg(OV7725_REG_COM5, 0x65);
      ov7725_write_reg(OV7725_REG_ADVFL, 0x00);
      ov7725_write_reg(OV7725_REG_ADVFH, 0x00);
      break;
    }
    case OV7725_LIGHT_MODE_HOME: {
      ov7725_write_reg(OV7725_REG_COM8, 0xFD);
      ov7725_write_reg(OV7725_REG_BLUE, 0x96);
      ov7725_write_reg(OV7725_REG_RED, 0x40);
      ov7725_write_reg(OV7725_REG_COM5, 0x65);
      ov7725_write_reg(OV7725_REG_ADVFL, 0x00);
      ov7725_write_reg(OV7725_REG_ADVFH, 0x00);
      break;
    }
    case OV7725_LIGHT_MODE_NIGHT: {
      ov7725_write_reg(OV7725_REG_COM8, 0xFF);
      ov7725_write_reg(OV7725_REG_COM5, 0xE5);
      break;
    }
    default: {
      return OV7725_EINVAL;
    }
  }

  return OV7725_EOK;
}

/**
 * @brief       设置OV7725模块色彩饱和度
 * @param       saturation: OV7725_COLOR_SATURATION_0: +4
 *                          OV7725_COLOR_SATURATION_1: +3
 *                          OV7725_COLOR_SATURATION_2: +2
 *                          OV7725_COLOR_SATURATION_3: +1
 *                          OV7725_COLOR_SATURATION_4: 0
 *                          OV7725_COLOR_SATURATION_5: -1
 *                          OV7725_COLOR_SATURATION_6: -2
 *                          OV7725_COLOR_SATURATION_7: -3
 *                          OV7725_COLOR_SATURATION_8: -4
 * @retval      OV7725_EOK   : 设置OV7725模块色彩饱和度成功
 *              OV7725_EINVAL: 传入参数错误
 */
uint8_t ov7725_set_color_saturation(OV7725ColorSaturation_t saturation) {
  switch (saturation) {
    case OV7725_COLOR_SATURATION_0: {
      ov7725_write_reg(OV7725_REG_USAT, 0x80);
      ov7725_write_reg(OV7725_REG_VSAT, 0x80);
      break;
    }
    case OV7725_COLOR_SATURATION_1: {
      ov7725_write_reg(OV7725_REG_USAT, 0x70);
      ov7725_write_reg(OV7725_REG_VSAT, 0x70);
      break;
    }
    case OV7725_COLOR_SATURATION_2: {
      ov7725_write_reg(OV7725_REG_USAT, 0x60);
      ov7725_write_reg(OV7725_REG_VSAT, 0x60);
      break;
    }
    case OV7725_COLOR_SATURATION_3: {
      ov7725_write_reg(OV7725_REG_USAT, 0x50);
      ov7725_write_reg(OV7725_REG_VSAT, 0x50);
      break;
    }
    case OV7725_COLOR_SATURATION_4: {
      ov7725_write_reg(OV7725_REG_USAT, 0x40);
      ov7725_write_reg(OV7725_REG_VSAT, 0x40);
      break;
    }
    case OV7725_COLOR_SATURATION_5: {
      ov7725_write_reg(OV7725_REG_USAT, 0x30);
      ov7725_write_reg(OV7725_REG_VSAT, 0x30);
      break;
    }
    case OV7725_COLOR_SATURATION_6: {
      ov7725_write_reg(OV7725_REG_USAT, 0x20);
      ov7725_write_reg(OV7725_REG_VSAT, 0x20);
      break;
    }
    case OV7725_COLOR_SATURATION_7: {
      ov7725_write_reg(OV7725_REG_USAT, 0x10);
      ov7725_write_reg(OV7725_REG_VSAT, 0x10);
      break;
    }
    case OV7725_COLOR_SATURATION_8: {
      ov7725_write_reg(OV7725_REG_USAT, 0x00);
      ov7725_write_reg(OV7725_REG_VSAT, 0x00);
      break;
    }
    default: {
      return OV7725_EINVAL;
    }
  }

  return OV7725_EOK;
}

/**
 * @brief       设置OV7725模块亮度
 * @param       brightness: OV7725_BRIGHTNESS_0: +4
 *                          OV7725_BRIGHTNESS_1: +3
 *                          OV7725_BRIGHTNESS_2: +2
 *                          OV7725_BRIGHTNESS_3: +1
 *                          OV7725_BRIGHTNESS_4: 0
 *                          OV7725_BRIGHTNESS_5: -1
 *                          OV7725_BRIGHTNESS_6: -2
 *                          OV7725_BRIGHTNESS_7: -3
 *                          OV7725_BRIGHTNESS_8: -4
 * @retval      OV7725_EOK   : 设置OV7725模块亮度成功
 *              OV7725_EINVAL: 传入参数错误
 */
uint8_t ov7725_set_brightness(OV7725Brightness_t brightness) {
  switch (brightness) {
    case OV7725_BRIGHTNESS_0: {
      ov7725_write_reg(OV7725_REG_BRIGHT, 0x48);
      ov7725_write_reg(OV7725_REG_SIGN, 0x06);
      break;
    }
    case OV7725_BRIGHTNESS_1: {
      ov7725_write_reg(OV7725_REG_BRIGHT, 0x38);
      ov7725_write_reg(OV7725_REG_SIGN, 0x06);
      break;
    }
    case OV7725_BRIGHTNESS_2: {
      ov7725_write_reg(OV7725_REG_BRIGHT, 0x28);
      ov7725_write_reg(OV7725_REG_SIGN, 0x06);
      break;
    }
    case OV7725_BRIGHTNESS_3: {
      ov7725_write_reg(OV7725_REG_BRIGHT, 0x18);
      ov7725_write_reg(OV7725_REG_SIGN, 0x06);
      break;
    }
    case OV7725_BRIGHTNESS_4: {
      ov7725_write_reg(OV7725_REG_BRIGHT, 0x08);
      ov7725_write_reg(OV7725_REG_SIGN, 0x06);
      break;
    }
    case OV7725_BRIGHTNESS_5: {
      ov7725_write_reg(OV7725_REG_BRIGHT, 0x08);
      ov7725_write_reg(OV7725_REG_SIGN, 0x0E);
      break;
    }
    case OV7725_BRIGHTNESS_6: {
      ov7725_write_reg(OV7725_REG_BRIGHT, 0x18);
      ov7725_write_reg(OV7725_REG_SIGN, 0x0E);
      break;
    }
    case OV7725_BRIGHTNESS_7: {
      ov7725_write_reg(OV7725_REG_BRIGHT, 0x28);
      ov7725_write_reg(OV7725_REG_SIGN, 0x0E);
      break;
    }
    case OV7725_BRIGHTNESS_8: {
      ov7725_write_reg(OV7725_REG_BRIGHT, 0x38);
      ov7725_write_reg(OV7725_REG_SIGN, 0x0E);
      break;
    }
    default: {
      return OV7725_EINVAL;
    }
  }

  return OV7725_EOK;
}

/**
 * @brief       设置OV7725模块对比度
 * @param       contrast: OV7725_CONTRAST_0: +4
 *                        OV7725_CONTRAST_1: +3
 *                        OV7725_CONTRAST_2: +2
 *                        OV7725_CONTRAST_3: +1
 *                        OV7725_CONTRAST_4: 0
 *                        OV7725_CONTRAST_5: -1
 *                        OV7725_CONTRAST_6: -2
 *                        OV7725_CONTRAST_7: -3
 *                        OV7725_CONTRAST_8: -4
 * @retval      OV7725_EOK   : 设置OV7725模块对比度成功
 *              OV7725_EINVAL: 传入参数错误
 */
uint8_t ov7725_set_contrast(OV7725Contrast_t contrast) {
  switch (contrast) {
    case OV7725_CONTRAST_0: {
      ov7725_write_reg(OV7725_REG_CNST, 0x30);
      break;
    }
    case OV7725_CONTRAST_1: {
      ov7725_write_reg(OV7725_REG_CNST, 0x2C);
      break;
    }
    case OV7725_CONTRAST_2: {
      ov7725_write_reg(OV7725_REG_CNST, 0x28);
      break;
    }
    case OV7725_CONTRAST_3: {
      ov7725_write_reg(OV7725_REG_CNST, 0x24);
      break;
    }
    case OV7725_CONTRAST_4: {
      ov7725_write_reg(OV7725_REG_CNST, 0x20);
      break;
    }
    case OV7725_CONTRAST_5: {
      ov7725_write_reg(OV7725_REG_CNST, 0x1C);
      break;
    }
    case OV7725_CONTRAST_6: {
      ov7725_write_reg(OV7725_REG_CNST, 0x18);
      break;
    }
    case OV7725_CONTRAST_7: {
      ov7725_write_reg(OV7725_REG_CNST, 0x14);
      break;
    }
    case OV7725_CONTRAST_8: {
      ov7725_write_reg(OV7725_REG_CNST, 0x10);
      break;
    }
    default: {
      return OV7725_EINVAL;
    }
  }

  return OV7725_EOK;
}

/**
 * @brief       设置OV7725模块特殊效果
 * @param       contrast: OV7725_SPECIAL_EFFECT_NORMAL  : Normal
 *                        OV7725_SPECIAL_EFFECT_BW      : B&W
 *                        OV7725_SPECIAL_EFFECT_BLUISH  : Bluish
 *                        OV7725_SPECIAL_EFFECT_SEPIA   : Sepia
 *                        OV7725_SPECIAL_EFFECT_REDISH  : Redish
 *                        OV7725_SPECIAL_EFFECT_GREENISH: Greenish
 *                        OV7725_SPECIAL_EFFECT_NEGATIE : Negative
 * @retval      OV7725_EOK   : 设置OV7725模块特殊效果成功
 *              OV7725_EINVAL: 传入参数错误
 */
uint8_t ov7725_set_special_effect(OV7725SpecialEffect_t effect) {
  switch (effect) {
    case OV7725_SPECIAL_EFFECT_NORMAL: {
      ov7725_write_reg(OV7725_REG_SDE, 0x06);
      ov7725_write_reg(OV7725_REG_UFix, 0x80);
      ov7725_write_reg(OV7725_REG_VFix, 0x80);
      break;
    }
    case OV7725_SPECIAL_EFFECT_BW: {
      ov7725_write_reg(OV7725_REG_SDE, 0x26);
      ov7725_write_reg(OV7725_REG_UFix, 0x80);
      ov7725_write_reg(OV7725_REG_VFix, 0x80);
      break;
    }
    case OV7725_SPECIAL_EFFECT_BLUISH: {
      ov7725_write_reg(OV7725_REG_SDE, 0x1E);
      ov7725_write_reg(OV7725_REG_UFix, 0xA0);
      ov7725_write_reg(OV7725_REG_VFix, 0x40);
      break;
    }
    case OV7725_SPECIAL_EFFECT_SEPIA: {
      ov7725_write_reg(OV7725_REG_SDE, 0x1E);
      ov7725_write_reg(OV7725_REG_UFix, 0x40);
      ov7725_write_reg(OV7725_REG_VFix, 0xA0);
      break;
    }
    case OV7725_SPECIAL_EFFECT_REDISH: {
      ov7725_write_reg(OV7725_REG_SDE, 0x1E);
      ov7725_write_reg(OV7725_REG_UFix, 0x80);
      ov7725_write_reg(OV7725_REG_VFix, 0xC0);
      break;
    }
    case OV7725_SPECIAL_EFFECT_GREENISH: {
      ov7725_write_reg(OV7725_REG_SDE, 0x1E);
      ov7725_write_reg(OV7725_REG_UFix, 0x60);
      ov7725_write_reg(OV7725_REG_VFix, 0x60);
      break;
    }
    case OV7725_SPECIAL_EFFECT_NEGATIVE: {
      ov7725_write_reg(OV7725_REG_SDE, 0x46);
      break;
    }
    default: {
      return OV7725_EINVAL;
    }
  }

  return OV7725_EOK;
}

/**
 * @brief       设置OV7725模块输出模式
 * @param       width : 输出图像宽度（VGA，<=640；QVGA，<=320）
 *              height: 输出图像高度（VGA，<=480；QVGA，<=240）
 *              mode  : OV7725_OUTPUT_MODE_VGA : VGA输出模式
 *                      OV7725_OUTPUT_MODE_QVGA: QVGA输出模式
 * @retval      OV7725_EOK   : 设置OV7725模块输出模式成功
 *              OV7725_EINVAL: 传入参数错误
 *              OV7725_ENOMEM: 内存不足
 */
uint8_t ov7725_set_output(uint16_t width, uint16_t height,
                          OV7725OutputMode_t mode) {
  uint16_t xs;
  uint16_t ys;
  uint8_t hstart_raw;
  uint8_t hstart_new;
  uint8_t vstrt_raw;
  uint8_t vstrt_new;
  uint8_t href_raw;
  uint8_t href_new;
  uint8_t exhch;

  switch (mode) {
    case OV7725_OUTPUT_MODE_VGA: {
      if ((width > OV7725_VGA_WIDTH_MAX) || (height > OV7725_VGA_HEIGHT_MAX)) {
        return OV7725_EINVAL;
      }
      break;
    }
    case OV7725_OUTPUT_MODE_QVGA: {
      if ((width > OV7725_QVGA_WIDTH_MAX) ||
          (height > OV7725_QVGA_HEIGHT_MAX)) {
        return OV7725_EINVAL;
      }
      break;
    }
    default: {
      return OV7725_EINVAL;
    }
  }

  switch (mode) {
    case OV7725_OUTPUT_MODE_VGA: {
      xs = (OV7725_VGA_WIDTH_MAX - width) >> 1;
      ys = (OV7725_VGA_HEIGHT_MAX - height) >> 1;
      ov7725_write_reg(OV7725_REG_COM7, 0x06);
      ov7725_write_reg(OV7725_REG_HSTART, 0x23);
      ov7725_write_reg(OV7725_REG_HSIZE, 0xA0);
      ov7725_write_reg(OV7725_REG_VSTRT, 0x07);
      ov7725_write_reg(OV7725_REG_VSIZE, 0xF0);
      ov7725_write_reg(OV7725_REG_HREF, 0x00);
      ov7725_write_reg(OV7725_REG_HOutSize, 0xA0);
      ov7725_write_reg(OV7725_REG_VOutSize, 0xF0);
      break;
    }
    case OV7725_OUTPUT_MODE_QVGA: {
      xs = (OV7725_QVGA_WIDTH_MAX - width) >> 1;
      ys = (OV7725_QVGA_HEIGHT_MAX - height) >> 1;
      ov7725_write_reg(OV7725_REG_COM7, 0x46);
      ov7725_write_reg(OV7725_REG_HSTART, 0x3F);
      ov7725_write_reg(OV7725_REG_HSIZE, 0x50);
      ov7725_write_reg(OV7725_REG_VSTRT, 0x03);
      ov7725_write_reg(OV7725_REG_VSIZE, 0x78);
      ov7725_write_reg(OV7725_REG_HREF, 0x00);
      ov7725_write_reg(OV7725_REG_HOutSize, 0x50);
      ov7725_write_reg(OV7725_REG_VOutSize, 0x78);
      break;
    }
    default: {
      return OV7725_EINVAL;
    }
  }

  hstart_raw = ov7725_read_reg(OV7725_REG_HSTART);
  hstart_new = hstart_raw + (xs >> 2);
  ov7725_write_reg(OV7725_REG_HSTART, hstart_new);
  ov7725_write_reg(OV7725_REG_HSIZE, width >> 2);
  g_ov7725_sta.output.width = ov7725_read_reg(OV7725_REG_HSIZE) << 2;

  vstrt_raw = ov7725_read_reg(OV7725_REG_VSTRT);
  vstrt_new = vstrt_raw + (ys >> 1);
  ov7725_write_reg(OV7725_REG_VSTRT, vstrt_new);
  ov7725_write_reg(OV7725_REG_VSIZE, height >> 1);
  g_ov7725_sta.output.height = ov7725_read_reg(OV7725_REG_VSIZE) << 1;

  href_raw = ov7725_read_reg(OV7725_REG_HREF);
  href_new = ((ys & 0x01) << 6) | ((xs & 0x03) << 4) | ((height & 0x01) << 2) |
             (width & 0x03) | href_raw;
  ov7725_write_reg(OV7725_REG_HREF, href_new);

  ov7725_write_reg(OV7725_REG_HOutSize, width >> 2);
  ov7725_write_reg(OV7725_REG_VOutSize, height >> 1);

  exhch = (href_raw | (width & 0x03) | ((height & 0x01) << 2));
  ov7725_write_reg(OV7725_REG_EXHCH, exhch);

  return OV7725_EOK;
}

/**
 * @brief       设置OV7725模块时钟预分频系数
 * @param       clk_rc: Internal clock pre-scalar（0~63）
 * @retval      OV7725_EOK   : 设置OV7725模块时钟预分频系数成功
 *              OV7725_EINVAL: 传入参数错误
 */
uint8_t ov7725_set_output_speed(uint8_t clk_rc) {
  if (clk_rc > 63) { return OV7725_EINVAL; }

  ov7725_write_reg(OV7725_REG_CLKRC, clk_rc);

  return OV7725_EOK;
}

/**
 * @brief       开启OV7725模块彩条测试
 * @param       无
 * @retval      无
 */
void ov7725_colorbar_enable(void) {
  uint8_t com3;

  com3 = ov7725_read_reg(OV7725_REG_COM3);
  com3 |= (uint8_t)(1 << 0);
  ov7725_write_reg(OV7725_REG_COM3, com3);
}

/**
 * @brief       关闭OV7725模块彩条测试
 * @param       无
 * @retval      无
 */
void ov7725_colorbar_disable(void) {
  uint8_t com3;

  com3 = ov7725_read_reg(OV7725_REG_COM3);
  com3 &= ~(uint8_t)(1 << 0);
  ov7725_write_reg(OV7725_REG_COM3, com3);
}

/**
 * @brief       获取OV7725模块输出图像宽度
 * @param       无
 * @retval      OV7725模块输出图像宽度
 */
uint16_t ov7725_get_output_width(void) { return g_ov7725_sta.output.width; }

/**
 * @brief       获取OV7725模块输出图像高度
 * @retval      OV7725模块输出图像高度
 */
uint16_t ov7725_get_output_height(void) { return g_ov7725_sta.output.height; }
