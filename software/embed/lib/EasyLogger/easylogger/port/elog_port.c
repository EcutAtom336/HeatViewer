/*
 * This file is part of the EasyLogger Library.
 *
 * Copyright (c) 2015, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for each platform.
 * Created on: 2015-04-28
 */

#include <elog.h>
/***/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
/***/
#include "gd32f4xx.h"
/***/
#include <stdio.h>

static SemaphoreHandle_t elog_mutex_handle __attribute__((section(".DTCM")));
static StaticSemaphore_t elog_mutex __attribute__((section(".DTCM")));

static SemaphoreHandle_t elog_async_mutex_handle
    __attribute__((section(".DTCM")));
static StaticSemaphore_t elog_async_mutex __attribute__((section(".DTCM")));

static TaskHandle_t elog_async_task_handle __attribute__((section(".DTCM")));
static StaticTask_t elog_async_task __attribute__((section(".DTCM")));
static uint8_t elog_async_task_stack[2048] __attribute__((section(".DTCM")));

/**
 * EasyLogger port initialize
 *
 * @return result
 */
ElogErrCode elog_port_init(void) {
#ifdef ELOG_ASYNC_OUTPUT_ENABLE
  extern ElogErrCode my_elog_async_init();
#endif  // ELOG_ASYNC_OUTPUT_ENABLE

  ElogErrCode result = ELOG_NO_ERR;

  rcu_periph_clock_enable(RCU_GPIOA);
  rcu_periph_clock_enable(RCU_USART0);
  // IO
  gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_9 | GPIO_PIN_10);
  gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ,
                          GPIO_PIN_9 | GPIO_PIN_10);
  gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_9 | GPIO_PIN_10);
  // usart0
  usart_baudrate_set(USART0, 115200);
  usart_stop_bit_set(USART0, USART_STB_1BIT);
  usart_parity_config(USART0, USART_PM_NONE);
  usart_word_length_set(USART0, USART_WL_8BIT);
  usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
  usart_enable(USART0);

  elog_mutex_handle = xSemaphoreCreateMutexStatic(&elog_mutex);
  configASSERT(elog_mutex_handle);

#ifdef ELOG_ASYNC_OUTPUT_ENABLE
  result = my_elog_async_init();
#endif  // ELOG_ASYNC_OUTPUT_ENABLE

  return result;
}

/**
 * EasyLogger port deinitialize
 *
 */
void elog_port_deinit(void) { /* add your code here */ }

/**
 * output log port interface
 *
 * @param log output of log
 * @param size log size
 */
void elog_port_output(const char *log, size_t size) {
  for (size_t i = 0; i < size; i++) {
    usart_data_transmit(USART0, *(log + i));
    while (!usart_flag_get(USART0, USART_FLAG_TC));
  }
}

/**
 * output lock
 */
void elog_port_output_lock(void) {
  xSemaphoreTake(elog_mutex_handle, portMAX_DELAY);
}

/**
 * output unlock
 */
void elog_port_output_unlock(void) { xSemaphoreGive(elog_mutex_handle); }

/**
 * get current time interface
 *
 * @return current time
 */
const char *elog_port_get_time(void) {
  static char time_str[16] = "";
  snprintf(time_str, 16, "%lu", (unsigned long)xTaskGetTickCount());
  return time_str;
}

/**
 * get current process name interface
 *
 * @return current process name
 */
const char *elog_port_get_p_info(void) { return ""; }

/**
 * get current thread name interface
 *
 * @return current thread name
 */
const char *elog_port_get_t_info(void) { return ""; }

#ifdef ELOG_ASYNC_OUTPUT_ENABLE
ElogErrCode my_elog_async_init() {
  void my_elog_async_output(void *arg);

  // elog_async_mutex_handle = xSemaphoreCreateMutexStatic(&elog_async_mutex);
  elog_async_mutex_handle =
      xSemaphoreCreateCountingStatic(UINT32_MAX, 1, &elog_async_mutex);
  configASSERT(elog_async_mutex_handle);

  configASSERT(!(sizeof(elog_async_task_stack) % 4));
  elog_async_task_handle = xTaskCreateStatic(
      my_elog_async_output, "elogAsync", sizeof(elog_async_task_stack) / 4,
      NULL, configMAX_PRIORITIES - 1, (StackType_t *)elog_async_task_stack,
      &elog_async_task);
  configASSERT(elog_async_task_handle);

  return ELOG_NO_ERR;
}

void elog_async_output_notice() { xSemaphoreGive(elog_async_mutex_handle); }

void my_elog_async_output(void *arg) {
  uint32_t log_size;

#ifdef ELOG_ASYNC_LINE_OUTPUT
  static char poll_get_buf[ELOG_LINE_BUF_SIZE - 4];
#else
  static char poll_get_buf[ELOG_ASYNC_OUTPUT_BUF_SIZE - 4];
#endif

  while (1) {
    xSemaphoreTake(elog_async_mutex_handle, portMAX_DELAY);
    while (1) {
#ifdef ELOG_ASYNC_LINE_OUTPUT
      log_size = elog_async_get_line_log(poll_get_buf, sizeof(poll_get_buf));
#else
      log_size = elog_async_get_log(poll_get_buf, sizeof(poll_get_buf));
#endif
      if (log_size)
        elog_port_output(poll_get_buf, log_size);
      else
        break;
    }
  }
}
#endif  // ELOG_ASYNC_OUTPUT_ENABLE
