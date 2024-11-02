#include "led.h"
/***/
#include "gd32f4xx.h"

const uint32_t LED_EN_PIN_PORT = GPIOE, LED_EN_PIN_NUM = GPIO_PIN_6;

void inline led_enable() { gpio_bit_set(LED_EN_PIN_PORT, LED_EN_PIN_NUM); }

void inline led_disable() { gpio_bit_reset(LED_EN_PIN_PORT, LED_EN_PIN_NUM); }

void inline led_toggle() { gpio_bit_toggle(LED_EN_PIN_PORT, LED_EN_PIN_NUM); }
