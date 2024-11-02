#ifndef _UTIL_IMAGE_H_
#define _UTIL_IMAGE_H_

#include <math.h>
#include <stdint.h>

#define U16_SWAP(x) (uint16_t)((((x) & 0xFF) << 8) + (((x) & 0xFF00) >> 8))

// typedef struct {
//   void *p_src, *p_dest;
//   uint32_t src_w, src_h, res_w, res_h;
// } UtilImageInfo_t;

void util_image_rgb565_zoom(const void *p_src, void *p_res,
                            const float_t zoom_ratio, const uint32_t x,
                            const uint32_t y);

void util_image_colour(void *frame, float_t *temp_frame, float_t *h_temp,
                       uint32_t *h_temp_idx, float_t *m_temp, float_t *l_temp,
                       uint32_t *l_temp_idx);

void util_image_rgb565grey_part_edge_detection(void *p_src_frame, void *p_dest_frame,
                                          uint16_t start_x, uint16_t start_y,
                                          uint16_t window_x, uint16_t window_y);

uint8_t util_image_save_rgb565_as_bmp(const void *p_frame);

#endif