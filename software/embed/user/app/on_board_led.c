#include "led.h"
///
#define LOG_TAG "led"
#include "elog.h"
//
#include "FreeRTOS.h"
#include "task.h"
///
#include "gd32f4xx.h"

static TaskHandle_t led_blink_handle __attribute__((section(".DTCM")));
static StaticTask_t led_blink_task __attribute__((section(".DTCM")));
static uint8_t led_blink_task_stack[256] __attribute__((section(".DTCM")));

static void on_board_led_blink(void *param) {
  while (1) {
    vTaskDelay(pdMS_TO_TICKS(500));
    gpio_bit_toggle(GPIOB, GPIO_PIN_2);
  }
}

void on_board_led_init() {
  configASSERT(!(sizeof(led_blink_task_stack) % 4));
  led_blink_handle = xTaskCreateStatic(
      on_board_led_blink, "ledBlink", sizeof(led_blink_task_stack) / 4, NULL, 0,
      (StackType_t *)led_blink_task_stack, &led_blink_task);
  configASSERT(led_blink_handle);

  log_i("init complete");
}
