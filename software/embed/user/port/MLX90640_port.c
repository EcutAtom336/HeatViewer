#define LOG_TAG "MLX90640 port"
#include "elog.h"
///
#include "MLX90640_I2C_Driver.h"
///
#include "gd32f4xx.h"
///
#include "FreeRTOS.h"
#include "task.h"
///
#include <stdint.h>

const uint32_t I2C_TIMEOUT_MS = 50;

const uint32_t MLX90640_I2C = I2C2;

void MLX90640_I2CInit() { return; }

int MLX90640_I2CGeneralReset() {
  uint32_t tick;

  log_v("i2c reset");

  tick = xTaskGetTickCount();
  while (i2c_flag_get(MLX90640_I2C, I2C_FLAG_I2CBSY)) {
    if (xTaskGetTickCount() - tick > pdMS_TO_TICKS(I2C_TIMEOUT_MS)) {
      log_e("i2c busy");
      return MLX90640_I2C_WRITE_ERROR;
    }
  }

  // i2c 开始信号
  i2c_start_on_bus(MLX90640_I2C);
  tick = xTaskGetTickCount();
  while (!i2c_flag_get(MLX90640_I2C, I2C_FLAG_SBSEND)) {
    if (xTaskGetTickCount() - tick > pdMS_TO_TICKS(I2C_TIMEOUT_MS)) {
      log_e("i2c start fail");
      return MLX90640_I2C_WRITE_ERROR;
    }
  }

  // 发送从机地址
  i2c_master_addressing(MLX90640_I2C, 0x00, I2C_TRANSMITTER);
  tick = xTaskGetTickCount();
  while (!i2c_flag_get(MLX90640_I2C, I2C_FLAG_ADDSEND)) {
    if (xTaskGetTickCount() - tick > pdMS_TO_TICKS(I2C_TIMEOUT_MS)) {
      log_e("i2c send 0x00 address fail");
      return MLX90640_I2C_WRITE_ERROR;
    }
  }
  i2c_flag_clear(MLX90640_I2C, I2C_FLAG_ADDSEND);

  i2c_data_transmit(MLX90640_I2C, 0x06);
  tick = xTaskGetTickCount();
  while (!i2c_flag_get(MLX90640_I2C, I2C_FLAG_BTC)) {
    if (xTaskGetTickCount() - tick > pdMS_TO_TICKS(I2C_TIMEOUT_MS)) {
      log_e("i2c send 0x06 addr fail");
      return MLX90640_I2C_WRITE_ERROR;
    }
  }

  i2c_stop_on_bus(MLX90640_I2C);
  tick = xTaskGetTickCount();
  while (I2C_CTL0(MLX90640_I2C) & I2C_CTL0_STOP) {
    if (xTaskGetTickCount() - tick > pdMS_TO_TICKS(I2C_TIMEOUT_MS)) {
      log_w("i2c stop timeout");
    }
  }

  return MLX90640_NO_ERROR;
}

void I2C_restore() {
  TickType_t tick;

  log_i("i2c restore");

  gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_8);
  gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
  for (size_t i = 0; i < 9; i++) {
    gpio_bit_set(GPIOA, GPIO_PIN_8);
    vTaskDelay(pdMS_TO_TICKS(1));
    gpio_bit_reset(GPIOA, GPIO_PIN_8);
    vTaskDelay(pdMS_TO_TICKS(1));
  }

  gpio_af_set(GPIOA, GPIO_AF_4, GPIO_PIN_8);
  gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_8);
  gpio_output_options_set(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_8);

  i2c_stop_on_bus(MLX90640_I2C);
  tick = xTaskGetTickCount();
  while (I2C_CTL0(MLX90640_I2C) & I2C_CTL0_STOP) {
    if (xTaskGetTickCount() - tick > pdMS_TO_TICKS(I2C_TIMEOUT_MS)) {
      log_w("i2c stop timeout");
      break;
    }
  }
}

int MLX90640_I2CRead(uint8_t slaveAddr, uint16_t startAddress,
                     uint16_t nMemAddressRead, uint16_t *data) {
  uint32_t tick;

  // log_v("i2c read, %u byte", nMemAddressRead << 1);

  tick = xTaskGetTickCount();
  while (i2c_flag_get(MLX90640_I2C, I2C_FLAG_I2CBSY)) {
    if (xTaskGetTickCount() - tick > pdMS_TO_TICKS(I2C_TIMEOUT_MS)) {
      log_e("i2c busy");
      I2C_restore();
      return MLX90640_I2C_WRITE_ERROR;
    }
  }

  // i2c 开始信号
  i2c_start_on_bus(MLX90640_I2C);
  tick = xTaskGetTickCount();
  while (!i2c_flag_get(MLX90640_I2C, I2C_FLAG_SBSEND)) {
    if (xTaskGetTickCount() - tick > pdMS_TO_TICKS(I2C_TIMEOUT_MS)) {
      log_e("i2c start fail");
      return MLX90640_I2C_WRITE_ERROR;
    }
  }

  // 发送从机地址
  i2c_master_addressing(MLX90640_I2C, slaveAddr, I2C_TRANSMITTER);
  tick = xTaskGetTickCount();
  while (!i2c_flag_get(MLX90640_I2C, I2C_FLAG_ADDSEND)) {
    if (xTaskGetTickCount() - tick > pdMS_TO_TICKS(I2C_TIMEOUT_MS)) {
      log_e("i2c send slave1 address fail");
      I2C_restore();
      return MLX90640_I2C_WRITE_ERROR;
    }
  }
  i2c_flag_clear(MLX90640_I2C, I2C_FLAG_ADDSEND);

  // 发送读地址
  i2c_data_transmit(MLX90640_I2C, startAddress >> 8);
  tick = xTaskGetTickCount();
  while (!i2c_flag_get(MLX90640_I2C, I2C_FLAG_TBE)) {
    if (xTaskGetTickCount() - tick > pdMS_TO_TICKS(I2C_TIMEOUT_MS)) {
      log_e("i2c send reg addr msb fail");
      I2C_restore();
      return MLX90640_I2C_WRITE_ERROR;
    }
  }
  i2c_data_transmit(MLX90640_I2C, startAddress & 0xFF);
  tick = xTaskGetTickCount();
  while (!i2c_flag_get(MLX90640_I2C, I2C_FLAG_BTC)) {
    if (xTaskGetTickCount() - tick > pdMS_TO_TICKS(I2C_TIMEOUT_MS)) {
      log_e("i2c send reg addr lsb fail");
      I2C_restore();
      return MLX90640_I2C_WRITE_ERROR;
    }
  }

  // i2c 开始信号
  i2c_start_on_bus(MLX90640_I2C);

  i2c_ackpos_config(MLX90640_I2C, 1 == nMemAddressRead ? I2C_ACKPOS_NEXT
                                                       : I2C_ACKPOS_CURRENT);

  tick = xTaskGetTickCount();
  while (!i2c_flag_get(MLX90640_I2C, I2C_FLAG_SBSEND)) {
    if (xTaskGetTickCount() - tick > pdMS_TO_TICKS(I2C_TIMEOUT_MS)) {
      log_e("i2c restart fail");
      I2C_restore();
      return MLX90640_I2C_WRITE_ERROR;
    }
  }

  // 再次发送从机地址
  i2c_master_addressing(MLX90640_I2C, slaveAddr, I2C_RECEIVER);

  tick = xTaskGetTickCount();
  while (!i2c_flag_get(MLX90640_I2C, I2C_FLAG_ADDSEND)) {
    if (xTaskGetTickCount() - tick > pdMS_TO_TICKS(I2C_TIMEOUT_MS)) {
      log_e("i2c send slave2 address fail");
      I2C_restore();
      return MLX90640_I2C_WRITE_ERROR;
    }
  }

  if (1 == nMemAddressRead) i2c_ack_config(MLX90640_I2C, I2C_ACK_DISABLE);

  i2c_flag_clear(MLX90640_I2C, I2C_FLAG_ADDSEND);

  // if (1 == nMemAddressRead) { i2c_stop_on_bus(MLX90640_I2C); }

  // 接收数据

  // new
  if (1 == nMemAddressRead) {
    tick = xTaskGetTickCount();
    while (!i2c_flag_get(MLX90640_I2C, I2C_FLAG_BTC)) {
      if (xTaskGetTickCount() - tick > pdMS_TO_TICKS(I2C_TIMEOUT_MS)) {
        I2C_restore();
        return MLX90640_I2C_WRITE_ERROR;
      }
    }
    i2c_stop_on_bus(MLX90640_I2C);
    *((uint8_t *)(data) + 1) = i2c_data_receive(MLX90640_I2C);
    *((uint8_t *)(data))     = i2c_data_receive(MLX90640_I2C);
  } else {
    for (size_t i = 0; i < nMemAddressRead; i++) {
      // msb
      tick = xTaskGetTickCount();
      while (!i2c_flag_get(MLX90640_I2C, I2C_FLAG_RBNE)) {
        if (xTaskGetTickCount() - tick > pdMS_TO_TICKS(I2C_TIMEOUT_MS)) {
          log_e("i2c recv %u byte fail", (i << 2) + 1);
          I2C_restore();
          return MLX90640_I2C_WRITE_ERROR;
        }
      }

      *((uint8_t *)(data) + (i << 1) + 1) = i2c_data_receive(MLX90640_I2C);

      if (2 == nMemAddressRead - i) {
        tick = xTaskGetTickCount();
        while (!i2c_flag_get(MLX90640_I2C, I2C_FLAG_BTC)) {
          if (xTaskGetTickCount() - tick > pdMS_TO_TICKS(I2C_TIMEOUT_MS)) {
            log_e("i2c recv %u byte fail", (i << 2) + 1);
            I2C_restore();
            return MLX90640_I2C_WRITE_ERROR;
          }
        }
        i2c_ack_config(MLX90640_I2C, I2C_ACK_DISABLE);
      }

      // lsb
      tick = xTaskGetTickCount();
      while (!i2c_flag_get(MLX90640_I2C, I2C_FLAG_RBNE)) {
        if (xTaskGetTickCount() - tick > pdMS_TO_TICKS(I2C_TIMEOUT_MS)) {
          log_e("i2c recv %u byte fail", (i << 2) + 2);
          I2C_restore();
          return MLX90640_I2C_WRITE_ERROR;
        }
      }
      *((uint8_t *)(data) + (i << 1)) = i2c_data_receive(MLX90640_I2C);
    }
  }

  // old
  // for (size_t i = 1; i <= byte_count; i++) {
  //   tick = xTaskGetTickCount();
  //   while (!i2c_flag_get(MLX90640_I2C, I2C_FLAG_RBNE)) {
  //     if (xTaskGetTickCount() - tick > pdMS_TO_TICKS(I2C_TIMEOUT_MS)) {
  //       log_e("%u byte, i2c recv fail", i);
  //       return MLX90640_I2C_WRITE_ERROR;
  //     }
  //   }

  //   if (i == byte_count - 2) {
  //     while (!i2c_flag_get(MLX90640_I2C, I2C_FLAG_BTC));
  //     i2c_ack_config(MLX90640_I2C, I2C_ACK_DISABLE);
  //   }

  //   if (i & 1)
  //     *(uint8_t *)(data + i) = i2c_data_receive(MLX90640_I2C);
  //   else
  //     *(uint8_t *)(data + i - 2) = i2c_data_receive(MLX90640_I2C);
  // }

  // i2c 停止信号

  i2c_stop_on_bus(MLX90640_I2C);
  tick = xTaskGetTickCount();
  while (I2C_CTL0(MLX90640_I2C) & I2C_CTL0_STOP) {
    if (xTaskGetTickCount() - tick > pdMS_TO_TICKS(I2C_TIMEOUT_MS)) {
      log_w("i2c stop timeout");
      I2C_restore();
      return MLX90640_I2C_WRITE_ERROR;
    }
  }
  // while (i2c_flag_get(MLX90640_I2C, I2C_FLAG_I2CBSY));

  i2c_ack_config(MLX90640_I2C, I2C_ACK_ENABLE);

  return MLX90640_NO_ERROR;
}

int MLX90640_I2CWrite(uint8_t slaveAddr, uint16_t writeAddress, uint16_t data) {
  uint32_t tick;

  // log_d("i2c write");

  tick = xTaskGetTickCount();
  while (i2c_flag_get(MLX90640_I2C, I2C_FLAG_I2CBSY)) {
    if (xTaskGetTickCount() - tick > pdMS_TO_TICKS(I2C_TIMEOUT_MS)) {
      log_e("i2c busy");
      I2C_restore();
      return MLX90640_I2C_WRITE_ERROR;
    }
  }

  // i2c 开始信号
  i2c_start_on_bus(MLX90640_I2C);
  tick = xTaskGetTickCount();
  while (!i2c_flag_get(MLX90640_I2C, I2C_FLAG_SBSEND)) {
    if (xTaskGetTickCount() - tick > pdMS_TO_TICKS(I2C_TIMEOUT_MS)) {
      log_e("i2c start fail");
      I2C_restore();
      return MLX90640_I2C_WRITE_ERROR;
    }
  }

  // 发送从机地址
  i2c_master_addressing(MLX90640_I2C, slaveAddr, I2C_TRANSMITTER);
  tick = xTaskGetTickCount();
  while (!i2c_flag_get(MLX90640_I2C, I2C_FLAG_ADDSEND)) {
    if (xTaskGetTickCount() - tick > pdMS_TO_TICKS(I2C_TIMEOUT_MS)) {
      log_e("i2c send slave address fail");
      I2C_restore();
      return MLX90640_I2C_WRITE_ERROR;
    }
  }
  i2c_flag_clear(MLX90640_I2C, I2C_FLAG_ADDSEND);

  // 发送写地址
  i2c_data_transmit(MLX90640_I2C, writeAddress >> 8);
  tick = xTaskGetTickCount();
  while (!i2c_flag_get(MLX90640_I2C, I2C_FLAG_TBE)) {
    if (xTaskGetTickCount() - tick > pdMS_TO_TICKS(I2C_TIMEOUT_MS)) {
      log_e("i2c send write reg addr fail");
      I2C_restore();
      return MLX90640_I2C_WRITE_ERROR;
    }
  }
  i2c_data_transmit(MLX90640_I2C, writeAddress & 0xFF);
  tick = xTaskGetTickCount();
  while (!i2c_flag_get(MLX90640_I2C, I2C_FLAG_TBE)) {
    if (xTaskGetTickCount() - tick > pdMS_TO_TICKS(I2C_TIMEOUT_MS)) {
      log_e("i2c send write reg addr fail");
      I2C_restore();
      return MLX90640_I2C_WRITE_ERROR;
    }
  }

  // 发送写数据
  i2c_data_transmit(MLX90640_I2C, data >> 8);
  tick = xTaskGetTickCount();
  while (!i2c_flag_get(MLX90640_I2C, I2C_FLAG_TBE)) {
    if (xTaskGetTickCount() - tick > pdMS_TO_TICKS(I2C_TIMEOUT_MS)) {
      log_e("i2c data send fail");
      I2C_restore();
      return MLX90640_I2C_WRITE_ERROR;
    }
  }
  i2c_data_transmit(MLX90640_I2C, data & 0xFF);
  tick = xTaskGetTickCount();
  while (!i2c_flag_get(MLX90640_I2C, I2C_FLAG_BTC)) {
    if (xTaskGetTickCount() - tick > pdMS_TO_TICKS(I2C_TIMEOUT_MS)) {
      log_e("i2c data send fail");
      I2C_restore();
      return MLX90640_I2C_WRITE_ERROR;
    }
  }

  i2c_stop_on_bus(MLX90640_I2C);
  tick = xTaskGetTickCount();
  while (I2C_CTL0(MLX90640_I2C) & I2C_CTL0_STOP) {
    if (xTaskGetTickCount() - tick > pdMS_TO_TICKS(I2C_TIMEOUT_MS)) {
      log_w("i2c stop timeout");
      I2C_restore();
      return MLX90640_I2C_WRITE_ERROR;
    }
  }
  // while (i2c_flag_get(MLX90640_I2C, I2C_FLAG_I2CBSY));

  return MLX90640_NO_ERROR;
}

void MLX90640_I2CFreqSet(int freq) {
  log_d("set i2c freq to %d", freq);
  // i2c_clock_config(MLX90640_I2C, freq, I2C_DTCY_16_9);
}
