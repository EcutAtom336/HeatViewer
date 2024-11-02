#include "laser.h"
/***/
#include "gd32f4xx.h"

const uint32_t LASER_EN_PIN_PORT = GPIOE, LASER_EN_PIN_NUM = GPIO_PIN_5;

void inline laser_enable() {
  gpio_bit_set(LASER_EN_PIN_PORT, LASER_EN_PIN_NUM);
}

void inline laser_disable() {
  gpio_bit_reset(LASER_EN_PIN_PORT, LASER_EN_PIN_NUM);
}

void inline laser_toggle() {
  gpio_bit_toggle(LASER_EN_PIN_PORT, LASER_EN_PIN_NUM);
}
