#include "util_image.h"
///
#include "bmp.h"
#include "color.h"
///
#include "FreeRTOS.h"
///
#include "gd32f4xx.h"
///
#include "ff.h"
///
#define LOG_TAG "util image"
#include "elog.h"
///
#include "arm_math_f16.h"
///
#include <stdio.h>
#include <math.h>

#define GET_RGB565_PIXEL(p, x, y, width) ((uint16_t *)p)[(x) + (y) * (width)]
#define GET_GREY_PIXEL(p, x, y, width)   ((uint8_t *)p)[(x) + (y) * (width)]

static inline uint8_t temp_map(float_t min, float_t max, float_t temp) {
  return (uint8_t)roundf(temp) < min
             ? (0)
             : (temp > max ? (255) : ((temp - min) / (max - min) * 255));
}

void util_image_rgb565_zoom(const void *p_src, void *p_res,
                            const float_t zoom_ratio, const uint32_t x_ofs,
                            const uint32_t y_ofs) {
  // 固定的图像尺寸
  const uint32_t IMG_H = 128, IMG_W = 160;

  if (zoom_ratio == 1.0f) {
    return;
  } else if (zoom_ratio < 1.0f) {
    // 缩小图像
    for (size_t h = 0; h < IMG_H; h++)
      for (size_t w = 0; w < IMG_W; w++) {
        if (h < y_ofs || w < x_ofs ||
            h >= y_ofs + (uint32_t)roundf(IMG_H * zoom_ratio) ||
            w >= x_ofs + (uint32_t)roundf(IMG_W * zoom_ratio)) {
          // 缩放后的空白区域，填零
          ((uint16_t *)p_res)[h * IMG_W + w] = 0;
        } else {
          ((uint16_t *)p_res)[h * IMG_W + w] =  //
              ((uint16_t *)
                   p_src)[(uint32_t)roundf((h - y_ofs) / zoom_ratio) * IMG_W +
                          (uint32_t)roundf((w - x_ofs) / zoom_ratio)];
        }
      }
  } else {
    // 放大图像，未实现
#warning "uncomplete"
  }
}

void util_image_colour(void *frame, float_t *temp_frame, float_t *P_h_temp,
                       uint32_t *p_h_temp_idx, float_t *p_c_temp,
                       float_t *p_l_temp, uint32_t *p_l_temp_idx) {
  const uint32_t W = 160, H = 128;

  float_t cur_temp,  // 当前像素温度
      r_temp,        // 相对当前像素右侧像素温度
      d_temp,        // 相对当前像素下方像素温度
      rd_temp;       // 相对当前像素右下像素温度

  float_t h_temp, l_temp;

  uint16_t cur_gray, r_gray, d_gray, rd_gray;

  // 找出温度极值
  arm_max_f32(temp_frame, 32 * 24, &h_temp, p_h_temp_idx);
  arm_min_f32(temp_frame, 32 * 24, &l_temp, p_l_temp_idx);

  *p_l_temp = l_temp;
  *p_c_temp = temp_frame[11 * 32 + 15];
  *P_h_temp = h_temp;

  if (h_temp - l_temp < 5.0f) h_temp = l_temp + 5.0f;

  // 热成像传感器输出数据是从右到左，从上到下的扫描方式
  // 图像扫描方式是从左到右，从上到下
  for (size_t temp_x = 0; temp_x < 32; temp_x++)
    for (size_t temp_y = 0; temp_y < 24; temp_y++) {
      // 读取温度
      cur_temp = temp_frame[temp_y * 32 + (31 - temp_x)];
      r_temp   = temp_x == 31 ? cur_temp
                              : temp_frame[(temp_y * 32) + ((31 - temp_x) - 1)];
      d_temp   = temp_y == 23 ? cur_temp
                              : temp_frame[((temp_y + 1) * 32) + (31 - temp_x)];
      rd_temp  = temp_x == 31 || temp_y == 24
                     ? cur_temp
                     : temp_frame[((temp_y + 1) * 32) + (31 - temp_x) - 1];

      // 归一化
      cur_gray = temp_map(l_temp, h_temp, cur_temp);
      r_gray   = temp_map(l_temp, h_temp, r_temp);
      d_gray   = temp_map(l_temp, h_temp, d_temp);
      rd_gray  = temp_map(l_temp, h_temp, rd_temp);

      for (size_t visible_x = temp_x * W / 32;
           visible_x < (temp_x + 1) * W / 32; visible_x++)
        for (size_t visible_y = temp_y * H / 24;
             visible_y < (temp_y + 1) * H / 24; visible_y++) {
          // 跳过已有颜色的像素
          if (((uint16_t *)frame)[visible_y * 160 + visible_x] !=
              RGB565(0, 0, 0))
            continue;

          if (visible_x < (temp_x + 1) * 160 / 32 - 160 / 32 / 2 &&
              visible_y < (temp_y + 1) * 128 / 24 - 128 / 24 / 2) {
            ((uint16_t *)frame)[visible_y * 160 + visible_x] =
                RAINBOW[cur_gray];
          } else if (visible_x >= (temp_x + 1) * 160 / 32 - 160 / 32 / 2 &&
                     visible_y >= (temp_y + 1) * 128 / 24 - 128 / 24 / 2) {
            ((uint16_t *)frame)[visible_y * 160 + visible_x] =
                RAINBOW[(cur_gray + r_gray + d_gray + rd_gray) / 4];
          } else if (visible_x >= (temp_x + 1) * 160 / 32 - 160 / 32 / 2) {
            ((uint16_t *)frame)[visible_y * 160 + visible_x] =
                RAINBOW[(cur_gray + r_gray) / 2];
          } else {
            ((uint16_t *)frame)[visible_y * 160 + visible_x] =
                RAINBOW[(cur_gray + d_gray) / 2];
          }
        }
    }
}

void util_image_rgb565grey_part_edge_detection(
    void *p_src_frame, void *p_dest_frame, uint16_t start_x, uint16_t start_y,
    uint16_t window_x, uint16_t window_y) {
  const uint32_t IMG_H = 128, IMG_W = 160;

  // Sobel 算子
  const int32_t SOBEL_OPERATOR_GX_3[3][3] = {
      {-1, 0, 1},
      {-2, 0, 2},
      {-1, 0, 1},
  };
  const int32_t SOBEL_OPERATOR_GY_3[3][3] = {
      {-1, -2, -1},
      {0, 0, 0},
      {1, 2, 1},
  };

  int32_t gx, gy;
  uint8_t grey;

  for (int32_t y = start_y; y < start_y + window_y; y++)
    for (int32_t x = start_x; x < start_x + window_x; x++) {
      gx = 0;
      gy = 0;

      if ((x % 2 || y % 2) || (y <= start_y || y >= start_y + window_y - 1 ||
                               x <= start_x || x >= start_x + window_x - 1)) {
        ((uint16_t *)p_dest_frame)[y * IMG_W + x] = 0;
        continue;
      }
      for (int32_t i = -1; i <= 1; i++)
        for (int32_t j = -1; j <= 1; j++) {
          grey = ((((uint16_t *)p_src_frame)[(y + i) * IMG_W + (x + j)] >> 3) &
                  0xFC);
          gx += SOBEL_OPERATOR_GX_3[i + 1][j + 1] * grey;
          gy += SOBEL_OPERATOR_GY_3[i + 1][j + 1] * grey;
        }

      uint8_t gradient = (uint8_t)sqrt(gx * gx + gy * gy);

      if (gradient < 32)
        gradient = 0;
      else if (gradient > 222)
        gradient = 255;

      // // 计算梯度幅值
      ((uint16_t *)p_dest_frame)[y * IMG_W + x] =
          RGB565(gradient, gradient, gradient);
    }
}

uint8_t util_image_save_rgb565_as_bmp(const void *p_frame) {
  const uint16_t H = 128, W = 160;

  char file_name[17];
  uint32_t file_num = 0;

  FIL f = {0};

  void *p_save_buf = NULL;

  uint32_t write_size = 0;

  FRESULT fatfs_ret = FR_OK;

  int8_t ret = -1;

  const uint32_t SAVE_BUF_SIZE = 1024;

  // 打开文件
  do {
    sprintf(file_name, "HV_%u.bmp", file_num++);
    fatfs_ret = f_open(&f, file_name, FA_CREATE_NEW | FA_WRITE);
  } while (fatfs_ret == FR_EXIST);
  if (fatfs_ret != FR_OK) {
    log_i("create file fail, code: %d", fatfs_ret);
    return -1;
  }

  p_save_buf = pvPortMalloc(SAVE_BUF_SIZE);
  if (p_save_buf == NULL) {
    log_w("save buf malloc fail.");
    return -1;
  }

  // 固定的 bmp 文件头
  const BmpFileHeader_t BMP_FILE_HEADER = {
      .bfType = 0x4D42,
      .bfSize =
          sizeof(BmpFileHeader_t) + sizeof(BmpInfoHeader_t) + 128 * 160 * 2,
      .bfReserved1 = 0,
      .bfReserved2 = 0,
      .bfOffBits   = sizeof(BmpFileHeader_t) + sizeof(BmpInfoHeader_t),
  };
  // 固定的 bmp 信息头
  const BmpInfoHeader_t BMP_INFO_HEADER = {
      .biSize          = sizeof(BmpInfoHeader_t),
      .biWidth         = W,
      .biHeight        = H,
      .biPlanes        = 1,
      .biBitCount      = 16,
      .biCompression   = 0,
      .biSizeImage     = 128 * 160 * 2,
      .biXPelsPerMeter = 100,
      .biYPelsPerMeter = 100,
      .biClrUsed       = 0,
      .biClrImportant  = 0,
  };

  // 写入 bmp 文件头
  fatfs_ret =
      f_write(&f, &BMP_FILE_HEADER, sizeof(BMP_FILE_HEADER), &write_size);
  if (fatfs_ret != FR_OK) {
    log_i("write bmp file header fail, code: %u", fatfs_ret);
    goto close;
  }
  log_i("write bmp file header success");

  // 写入 bmp 信息头
  fatfs_ret =
      f_write(&f, &BMP_INFO_HEADER, sizeof(BMP_INFO_HEADER), &write_size);
  if (fatfs_ret != FR_OK) {
    log_i("write bmp info header fail, code: %u", fatfs_ret);
    goto close;
  }
  log_i("write bmp info header success");

  uint32_t buf_cnt = 0, row_pixel_cnt = 0;

  // bmp 要求每行像素字节数为 4 字节对齐
  //  一行像素所占的大小还差多少字节到 4 字节对齐
  uint8_t row_byte_to_align_4byte = 4 - (W * 2 % 4 == 0 ? 4 : W * 2 % 4);

  uint16_t cur_pixel_raw, cur_pixel_bmp;

  for (size_t frame_pixel_cnt = 0; frame_pixel_cnt <= W * H;
       frame_pixel_cnt++) {
    // 读取当前像素
    // 输入图像扫描方式是从左到右从上到下，bmp 图像扫描方式是从左到右从下到上
    // 保存的是 lcd 显存，要翻转高低字节
    cur_pixel_raw = U16_SWAP(
        ((uint16_t *)p_frame)[(W * H) - ((frame_pixel_cnt + W) / W) * W +
                              (frame_pixel_cnt % W)]);

    // 将 rgb565 像素转换为 bmp 像素
    cur_pixel_bmp = ((cur_pixel_raw & 0xF800) >> 1) +
                    ((cur_pixel_raw & 0x7C0) >> 1) + (cur_pixel_raw & 0x1F);

    ((uint8_t *)p_save_buf)[++buf_cnt - 1] = cur_pixel_bmp & 0xFF;
    ((uint8_t *)p_save_buf)[++buf_cnt - 1] = (cur_pixel_bmp >> 8) & 0xFF;
    row_pixel_cnt++;

    if (row_pixel_cnt == W) {
      // 行结束，不会触发写入
      if (row_byte_to_align_4byte) {
        // 行数据大小不是 4 的倍数
        // buf 剩余的的大小一定够将行像素大小补到 4 的倍数
        for (size_t i = 0; i < row_byte_to_align_4byte; i++)
          ((uint8_t *)p_save_buf)[++buf_cnt - 1] = 0;
      }
      row_pixel_cnt = 0;
    }

    if (buf_cnt > sizeof(SAVE_BUF_SIZE) - row_byte_to_align_4byte - 2 * 2) {
      // buf 满
      fatfs_ret = f_write(&f, p_save_buf, buf_cnt, &write_size);
      if (fatfs_ret != FR_OK) {
        log_i("save bmp fail, code: %u", fatfs_ret);
        goto close;
      }
      buf_cnt = 0;
    }

    if (frame_pixel_cnt == W * H) {
      if (row_byte_to_align_4byte) {
        // 行数据大小不是 4 的倍数
        // buf 剩余的的大小一定够将行像素大小补到 4 的倍数
        for (size_t i = 0; i < row_byte_to_align_4byte; i++)
          ((uint8_t *)p_save_buf)[++buf_cnt - 1] = 0;
      }
      fatfs_ret = f_write(&f, p_save_buf, buf_cnt, &write_size);
      if (fatfs_ret != FR_OK) {
        log_i("save bmp fail, code: %u", fatfs_ret);
        goto close;
      }
    }
  }

  log_i("write bmp success");

  ret = 0;

close:
  vPortFree(p_save_buf);
  fatfs_ret = f_close(&f);
  if (fatfs_ret != FR_OK) { log_w("close file fail, code; %u", fatfs_ret); }

  return ret;
}
