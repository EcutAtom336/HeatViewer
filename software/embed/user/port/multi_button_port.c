#include "multi_button_port.h"
/***/
#define LOG_TAG "multi button port"
#include "elog.h"
//
#include "multi_button.h"
//
#include "FreeRTOS.h"
#include "task.h"
/***/
#include "gd32f4xx.h"
/***/
#include <stdbool.h>

static TaskHandle_t btn_handler_task_handle __attribute__((section(".DTCM")));
static StaticTask_t btn_handler_task __attribute__((section(".DTCM")));
static uint8_t btn_handler_task_stack[2048] __attribute__((section(".DTCM")));

uint8_t multi_button_read_btn(uint8_t btn) {
  const uint32_t BTN_PIN_PORT_TABLE[BtnMax] = {
      [BtnPower]   = GPIOE,  //
      [BtnMenu]    = GPIOE,  //
      [BtnUp]      = GPIOE,  //
      [BtnDown]    = GPIOA,  //
      [BtnTrigger] = GPIOC,  //
  };
  const uint32_t BTN_PIN_NUM_TABLE[BtnMax] = {
      [BtnPower]   = GPIO_PIN_7,  //
      [BtnMenu]    = GPIO_PIN_3,  //
      [BtnUp]      = GPIO_PIN_2,  //
      [BtnDown]    = GPIO_PIN_2,  //
      [BtnTrigger] = GPIO_PIN_0,  //
  };

  return gpio_input_bit_get(BTN_PIN_PORT_TABLE[btn], BTN_PIN_NUM_TABLE[btn]) ==
                 RESET
             ? 0
             : 1;
}

static void multi_button_handler(void *param) {
  while (1) {
    button_ticks();
    vTaskDelay(pdMS_TO_TICKS(TICKS_INTERVAL));
  }
}

void multi_button_port_init() {
  configASSERT(!(sizeof(btn_handler_task_stack) % 4));
  btn_handler_task_handle = xTaskCreateStatic(
      multi_button_handler, "btnHandler", sizeof(btn_handler_task_stack) / 4,
      NULL, 12, (StackType_t *)btn_handler_task_stack, &btn_handler_task);
  configASSERT(btn_handler_task_handle);

  log_i("init complete");
}
