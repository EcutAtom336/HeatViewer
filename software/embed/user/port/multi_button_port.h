#ifndef _MULTI_BUTTON_PORT_H_
#define _MULTI_BUTTON_PORT_H_

#include <stdint.h>

typedef enum {
  BtnPower,    // 有效电平：0
  BtnMenu,     // 有效电平：0
  BtnUp,       // 有效电平：0
  BtnDown,     // 有效电平：0
  BtnTrigger,  // 有效电平：0
  BtnMax,
} BtnNum_t;

void multi_button_port_init();

uint8_t multi_button_read_btn(uint8_t btn);

#endif