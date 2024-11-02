#include "power_manage.h"
///
#define LOG_TAG "power manage"
#include "elog.h"
///
#include "buzzer.h"
#include "multi_button_port.h"
///
#include "FreeRTOS.h"
#include "semphr.h"
///
#include "multi_button.h"
///
#include "gd32f4xx.h"
///
#include <math.h>

static Button __attribute__((section(".DTCM"))) btn_pwr;

static void power_manage_off() {
  buzzer_sing(BuzzerToneFail);

  gpio_bit_reset(GPIOC, GPIO_PIN_13);
}

void power_manage_init() {
  gpio_bit_set(GPIOC, GPIO_PIN_13);

  button_init(&btn_pwr, multi_button_read_btn, 0, BtnPower);
  button_attach(&btn_pwr, LONG_PRESS_START, power_manage_off);
  button_start(&btn_pwr);
}

uint8_t power_manage_is_charge() {
  return gpio_input_bit_get(GPIOA, GPIO_PIN_1) == SET ? 1 : 0;
}

BatteryLevel_t power_manage_get_battery_status() {
  const float_t DIV_R_H = 10000.0F, DIV_R_L = 22000.0F, VOL_H = 3.9F,
                VOL_M = 3.6F, VOL_L = 3.2F;

  uint16_t adc_raw;

  float_t voltage;

  if (power_manage_is_charge()) return BatteryLevelCharge;

  adc_software_trigger_enable(ADC0, ADC_ROUTINE_CHANNEL);

  TickType_t tick = xTaskGetTickCount();
  while (adc_flag_get(ADC0, ADC_FLAG_EOC) == RESET) {
    if (xTaskGetTickCount() - tick > pdMS_TO_TICKS(1)) {
      log_w("adc convert fail.");
      return BatteryLevelMid;
    }
  }

  adc_raw = adc_routine_data_read(ADC0);

  voltage = ((float_t)adc_raw / 4095.0F * 3.3F) / DIV_R_L * (DIV_R_H + DIV_R_L);

  // log_d("battery voltage: %.2f", voltage);

  if (voltage >= VOL_H)
    return BatteryLevelHigh;
  else if (voltage >= VOL_M)
    return BatteryLevelMid;
  else if (voltage >= VOL_L)
    return BatteryLevelLow;
  else
    return BatteryLevelUrgent;
}
