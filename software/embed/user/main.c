#include "buzzer.h"
#include "hw_init.h"
#include "my_lcd.h"
#include "sw_init.h"
#include "mlx90640_handler.h"
#include "pic.h"
#include "app.h"
///
#define LOG_TAG "main"
#include "elog.h"
//
#include "FreeRTOS.h"
#include "task.h"
///
#include "cm_backtrace.h"
///
#include "systick.h"
#include "gd32f4xx.h"
///
#include <math.h>
#include <stddef.h>
#include <stdio.h>

static void init_code() {
  vTaskDelay(pdMS_TO_TICKS(500));

  int32_t ret;

  elog_init();
  elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
  elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
  elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
  elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
  elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
  elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
  elog_start();

  hw_init();

  sw_init();

  // 开机提示音
  buzzer_sing(BuzzerToneSuccess);

  // 显示 logo
  lcd_show_rgb565_pic(0, 0, LOGO_IMG_HEIGHT, LOGO_IMG_WIDTH,
                      (uint16_t*)LOGO_IMG);

  lcd_refreash_request();

  vTaskDelay(pdMS_TO_TICKS(1500));

  app_init();
  mlx90640_handler_init();

  vTaskDelete(NULL);
}

int main() {
  // 初始化 USART0
  // 使能时钟
  rcu_periph_clock_enable(RCU_GPIOA);
  rcu_periph_clock_enable(RCU_USART0);
  // IO 配置
  gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_9 | GPIO_PIN_10);
  gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ,
                          GPIO_PIN_9 | GPIO_PIN_10);
  gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_9 | GPIO_PIN_10);
  // 配置 USART0
  usart_baudrate_set(USART0, 115200);
  usart_stop_bit_set(USART0, USART_STB_1BIT);
  usart_parity_config(USART0, USART_PM_NONE);
  usart_word_length_set(USART0, USART_WL_8BIT);
  usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
  usart_enable(USART0);

  // cm backtrace 初始化
  cm_backtrace_init("HeatViewer", "v1.0.0", "v1.0.0");

  BaseType_t freertos_ret;

  nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);

  freertos_ret = xTaskCreate(init_code, "init", 1024 * 4 / 4, NULL,
                             configMAX_PRIORITIES - 1, NULL);
  configASSERT(freertos_ret);

  systick_config();

  vTaskStartScheduler();
}
