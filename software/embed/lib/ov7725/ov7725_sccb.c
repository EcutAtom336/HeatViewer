#include "ov7725_sccb.h"
#include "gd32f4xx_i2c.h"
///
#define LOG_TAG "ov7725 sccb"
#include "elog.h"
///
#include "FreeRTOS.h"
#include "task.h"
///
#include "gd32f4xx.h"

const uint32_t SCCB_SCL_PIN_PORT = GPIOD, SCCB_SCL_PIN_NUM = GPIO_PIN_5,
               SCCB_SDA_PIN_PORT = GPIOD, SCCB_SDA_PIN_NUM = GPIO_PIN_1;

const static uint32_t SCCB_DELAY_ARGS = 100;

static inline void sccb_wait() {
  for (size_t i = 0; i < SCCB_DELAY_ARGS; i++) __NOP();
}

static inline void sccb_scl_set() {
  gpio_bit_set(SCCB_SCL_PIN_PORT, SCCB_SCL_PIN_NUM);
}

static inline void sccb_scl_reset() {
  gpio_bit_reset(SCCB_SCL_PIN_PORT, SCCB_SCL_PIN_NUM);
}

static inline void sccb_sda_set() {
  gpio_mode_set(SCCB_SDA_PIN_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
                SCCB_SDA_PIN_NUM);
  gpio_bit_set(SCCB_SDA_PIN_PORT, SCCB_SDA_PIN_NUM);
}

static inline void sccb_sda_reset() {
  gpio_mode_set(SCCB_SDA_PIN_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
                SCCB_SDA_PIN_NUM);
  gpio_bit_reset(SCCB_SDA_PIN_PORT, SCCB_SDA_PIN_NUM);
}

static inline uint8_t sccb_sda_read() {
  gpio_mode_set(SCCB_SDA_PIN_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE,
                SCCB_SDA_PIN_NUM);
  return gpio_input_bit_get(SCCB_SDA_PIN_PORT, SCCB_SDA_PIN_NUM) == SET ? 1
                                                                         : 0;
}

static void sccb_start() {
  // sccb_scl_reset();
  // sccb_wait();
  // sccb_sda_set();
  // sccb_wait();
  // sccb_scl_set();
  // sccb_wait();
  // sccb_sda_reset();
  // sccb_wait();

  sccb_sda_set();
  sccb_wait();
  sccb_scl_set();
  sccb_wait();
  sccb_sda_reset();
  sccb_wait();
  sccb_scl_reset();
}

static void sccb_stop() {
  // sccb_scl_reset();
  // sccb_wait();
  // sccb_sda_reset();
  // sccb_wait();
  // sccb_scl_set();
  // sccb_wait();
  // sccb_sda_set();
  // sccb_wait();

  sccb_sda_reset();
  sccb_wait();
  sccb_scl_set();
  sccb_wait();
  sccb_sda_set();
  sccb_wait();
}

static void sccb_send(uint8_t dat) {
  // for (size_t i = 0; i < 8; i++) {
  //   sccb_scl_reset();
  //   sccb_wait();
  //   // dat & (0x80 >> i) ? sccb_sda_set() : sccb_sda_reset();
  //   dat >> (7 - i) & 1 ? sccb_sda_set() : sccb_sda_reset();
  //   sccb_wait();
  //   sccb_scl_set();
  //   sccb_wait();
  // }

  ///

  int8_t dat_index;
  uint8_t dat_bit;

  for (dat_index = 7; dat_index >= 0; dat_index--) {
    dat_bit = (dat >> dat_index) & 0x01;
    dat_bit ? sccb_sda_set() : sccb_sda_reset();
    sccb_wait();
    sccb_scl_set();
    sccb_wait();
    sccb_scl_reset();
  }

  sccb_sda_set();
  sccb_wait();
  sccb_scl_set();
  sccb_wait();
  sccb_scl_reset();
}

static void sccb_read(uint8_t *dat) {
  // for (size_t i = 0; i < 8; i++) {
  //   sccb_scl_reset();
  //   sccb_wait();
  //   sccb_scl_set();
  //   sccb_wait();
  //   *dat += sccb_sda_read() ? 0x80 >> i : 0;
  //   sccb_wait();
  // }

  int8_t dat_index;
  uint8_t dat_bit;

  for (dat_index = 7; dat_index >= 0; dat_index--) {
    sccb_wait();
    sccb_scl_set();
    dat_bit = sccb_sda_read();
    *dat |= (dat_bit << dat_index);
    sccb_wait();
    sccb_scl_reset();
  }

  sccb_wait();
  sccb_scl_set();
  sccb_wait();
  sccb_scl_reset();
  sccb_wait();
  sccb_sda_reset();
  sccb_wait();
}

/* SCCB接口读写通讯地址bit0 */
#define ATK_MC7725_SCCB_WRITE 0x00
#define ATK_MC7725_SCCB_READ  0x01

/**
 * @brief       SCCB接口3阶段写传输
 * @param       id_addr : ID Address
 *              sub_addr: Sub-address
 *              dat     : Write Data
 * @retval      无
 */
void ov7725_sccb_3_phase_write(uint8_t id_addr, uint8_t sub_addr, uint8_t dat) {
  sccb_start();
  sccb_send((id_addr << 1) | ATK_MC7725_SCCB_WRITE);
  sccb_send(sub_addr);
  sccb_send(dat);
  sccb_stop();
}

/**
 * @brief       SCCB接口2阶段写传输
 * @param       id_addr : ID Address
 *              sub_addr: Sub-address
 * @retval      无
 */
void ov7725_sccb_2_phase_write(uint8_t id_addr, uint8_t sub_addr) {
  sccb_start();
  sccb_send((id_addr << 1) | ATK_MC7725_SCCB_WRITE);
  sccb_send(sub_addr);
  sccb_stop();
}

/**
 * @brief       SCCB接口2阶段读传输
 * @param       id_addr: ID Address
 *              dat: 读取到的数据
 * @retval      无
 */
void ov7725_sccb_2_phase_read(uint8_t id_addr, uint8_t *dat) {
  sccb_start();
  sccb_send((id_addr << 1) | ATK_MC7725_SCCB_READ);
  sccb_read(dat);
  sccb_stop();
}
