#define __FILE_INCOMPLETE 1
///
#include "gd32f4xx.h"
///
#include <stdio.h>

// 告知连接器不从C库链接使用半主机的函数
__asm(".global __use_no_semihosting\n\t");

struct __FILE {
  int handle;
};

FILE __stdout;

void _sys_exit(int x) { x = x; }

/* 标准库需要的支持类型 */

// FILE __stdout;
int fputc(int ch, FILE* stream) {
  usart_data_transmit(USART0, ch);
  while (!usart_flag_get(USART0, USART_FLAG_TC));
  return ch;
}
