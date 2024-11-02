#include "buzzer.h"
//
#define LOG_TAG "buzzer"
#include "elog.h"
//
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
//
#include "gd32f4xx.h"
//
#include <stdbool.h>
#include <stddef.h>

static TaskHandle_t __attribute__((section(".DTCM"))) buzzer_task_handle;
static StaticTask_t __attribute__((section(".DTCM"))) buzzer_task;
static uint8_t __attribute__((section(".DTCM"))) buzzer_task_stack[1024];

static QueueHandle_t __attribute__((section(".DTCM"))) buzzer_queue_handle;
static StaticQueue_t __attribute__((section(".DTCM"))) buzzer_queue;
static uint8_t
    __attribute__((section(".DTCM"))) buzzer_queue_buf[sizeof(uint32_t) * 4];

static void buzzer_freq_set(uint16_t freq) {
  if (freq == 0) timer_disable(TIMER1);
  timer_parameter_struct timer_init_param;
  // timer_oc_parameter_struct timer_oc_init_param;
  if (freq < 800 || freq > 5200) return;
  timer_init_param.prescaler         = SystemCoreClock / 1U / 1000000U - 1U;
  timer_init_param.alignedmode       = TIMER_COUNTER_EDGE;
  timer_init_param.counterdirection  = TIMER_COUNTER_UP;
  timer_init_param.clockdivision     = TIMER_CKDIV_DIV4;
  timer_init_param.period            = 1000000U / freq - 1U;
  timer_init_param.repetitioncounter = 0;
  timer_init(TIMER1, &timer_init_param);
  timer_channel_output_mode_config(TIMER1, TIMER_CH_0, TIMER_OC_MODE_PWM0);
  timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_0,
                                          (1000000 / freq - 1) / 2);
  timer_enable(TIMER1);
}

static void buzzer_handler(void *prarm) {
  BuzzerToneNum_t tone_num;
  for (;;) {
    xQueueReceive(buzzer_queue_handle, &tone_num, portMAX_DELAY);
    if (tone_num == BuzzerToneSuccess) {
      buzzer_freq_set(3800);
      vTaskDelay(pdMS_TO_TICKS(200));
      buzzer_freq_set(4500);
      vTaskDelay(pdMS_TO_TICKS(200));
      buzzer_freq_set(5200);
      vTaskDelay(pdMS_TO_TICKS(200));
    } else if (tone_num == BuzzerToneFail) {
      buzzer_freq_set(2000);
      vTaskDelay(pdMS_TO_TICKS(500));
    } else if (tone_num == BuzzerToneWarning) {
      buzzer_freq_set(4000);
      vTaskDelay(pdMS_TO_TICKS(100));
      buzzer_freq_set(0);
      vTaskDelay(pdMS_TO_TICKS(100));
      buzzer_freq_set(4000);
      vTaskDelay(pdMS_TO_TICKS(100));
      buzzer_freq_set(0);
      vTaskDelay(pdMS_TO_TICKS(100));
      buzzer_freq_set(4000);
      vTaskDelay(pdMS_TO_TICKS(100));
    } else if (tone_num == BuzzerToneClick) {
      buzzer_freq_set(4000);
      vTaskDelay(pdMS_TO_TICKS(75));
    } else if (tone_num == BuzzerTone4khz500ms) {
      buzzer_freq_set(4000);
      vTaskDelay(pdMS_TO_TICKS(500));
    }
    buzzer_freq_set(0);
  }
}

void buzzer_init() {
  buzzer_queue_handle =
      xQueueCreateStatic(sizeof(buzzer_queue_buf) / sizeof(uint32_t),
                         sizeof(uint32_t), buzzer_queue_buf, &buzzer_queue);
  configASSERT(buzzer_queue_handle);

  buzzer_task_handle = xTaskCreateStatic(
      buzzer_handler, "buzzerHandler", sizeof(buzzer_task_stack) / 4, NULL, 6,
      (StackType_t *)buzzer_task_stack, &buzzer_task);
  configASSERT(buzzer_task_handle);

  log_i("init complete");
}

void buzzer_sing(BuzzerToneNum_t tone_num) {
  if (tone_num >= BuzzerToneMax) return;
  xQueueSendToBack(buzzer_queue_handle, &tone_num, 0);
}