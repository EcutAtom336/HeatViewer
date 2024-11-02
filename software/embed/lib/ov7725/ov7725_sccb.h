#ifndef _OV7725_SCCB_H_
#define _OV7725_SCCB_H_

#include "gd32f4xx.h"
///
#include <stdint.h>

void ov7725_sccb_3_phase_write(uint8_t id_addr, uint8_t sub_addr, uint8_t dat);
void ov7725_sccb_2_phase_write(uint8_t id_addr, uint8_t sub_addr);
void ov7725_sccb_2_phase_read(uint8_t id_addr, uint8_t *dat);

#endif
