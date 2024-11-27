#include "hw_init.h"
///
#include "MLX90640_repair.h"
#include "lcd_init.h"
#include "sd_card_ex.h"
#include "mlx90640_user.h"
//
#include "MLX90640_API.h"
#include "ov7725.h"
#include "sd_card.h"
//
#define LOG_TAG "hw init"
#include "elog.h"
//
#include "FreeRTOS.h"
#include "task.h"
///
#include "gd32f4xx.h"
///
#include <string.h>

/* IO distribute
 *
 * GPIO
 * LED            PB2
 * SD_DET         PD3
 * CHARGE_DET     PA1
 * 3V3_EN_MCU     PC13
 * LASER_EN       PE5
 * LED_EN         PE6
 * LCD_CS         PD11
 * LCD_BL         PD12
 * LCD_RESET      PD8
 * LCD_DC         PD10
 * BTN_PWR        PE7
 * BTN_MF1        PE2
 * BTN_MF2        PA2
 * BTN_MF3        PC0
 * BTN_MENU       PE3
 * OV7725_PWDN    PB5
 * OV7725_RESET   PD7
 * SCCB_SCL       PD5
 * SCCB_SDA       PD1
 *
 * SDIO
 * SDIO_D0        PC8
 * SDIO_D1        PC9
 * SDIO_D2        PC10
 * SDIO_D3        PC11
 * SDIO_CLK       PC12
 * SDIO_CMD       PD2
 *
 * DCI
 * DCI_HSYNC      PA4
 * DCI_VSYNC      PB7
 * DCI_PIXCLK     PA6
 * DCI_D0         PC6
 * DCI_D1         PC7
 * DCI_D2         PE0
 * DCI_D3         PE1
 * DCI_D4         PE4
 * DCI_D5         PB6
 * DCI_D6         PB8
 * DCI_D7         PB9
 *
 * SPI
 * SPI1_SCK       PB13
 * SPI1_MOSI      PB15
 *
 * I2C
 * I2C2_SCL       PA8
 * I2C2_SDA       PB4
 *
 * TIMER
 * T1CH0          PA15
 *
 * ADC
 * ADC_CH8        PB0
 */

/*
 * DMA
 * DCI SPI I2C
 */

static void gpio_init() {
  // LED
  // 开发板上 led
  rcu_periph_clock_enable(RCU_GPIOB);

  gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_2);
  gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_2);

  // SD DET
  rcu_periph_clock_enable(RCU_GPIOD);

  gpio_mode_set(GPIOD, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_3);

  // CHARGE DET
  rcu_periph_clock_enable(RCU_GPIOA);

  gpio_mode_set(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_1);

  // 3V3 EN MCU
  rcu_periph_clock_enable(RCU_GPIOC);

  gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_13);
  gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_13);

  // LASER EN
  rcu_periph_clock_enable(RCU_GPIOE);

  gpio_mode_set(GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_5);
  gpio_output_options_set(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_5);

  // LED EN
  rcu_periph_clock_enable(RCU_GPIOE);

  gpio_mode_set(GPIOE, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_6);
  gpio_output_options_set(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_6);

  // LCD control
  rcu_periph_clock_enable(RCU_GPIOD);

  gpio_mode_set(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
                GPIO_PIN_8 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12);
  gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ,
                          GPIO_PIN_8 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12);

  // BTN
  rcu_periph_clock_enable(RCU_GPIOA);
  rcu_periph_clock_enable(RCU_GPIOC);
  rcu_periph_clock_enable(RCU_GPIOE);

  gpio_mode_set(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_2);

  gpio_mode_set(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_0);

  gpio_mode_set(GPIOE, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP,
                GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_7);

  // OV7725 control
  rcu_periph_clock_enable(RCU_GPIOB);
  rcu_periph_clock_enable(RCU_GPIOD);

  gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_5);
  gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_5);
  gpio_bit_reset(GPIOB, GPIO_PIN_5);  // 使能 ov7725

  gpio_mode_set(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_7);
  gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_7);

  // SCCB
  rcu_periph_clock_enable(RCU_GPIOD);

  gpio_mode_set(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
                GPIO_PIN_1 | GPIO_PIN_5);
  gpio_output_options_set(GPIOD, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ,
                          GPIO_PIN_1 | GPIO_PIN_5);

  log_i("gpio init complete");
}

static void sdio_init() {
  sd_error_enum sd_ret;

  if (sd_card_get_phy_status() == SdCardNotExist) {
    log_i("sd card not found.");
    return;
  }

  sd_ret = sd_init();
  if (sd_ret != SD_OK) {
    log_w("sd init fail. code: %d", sd_ret);
    return;
  }

  log_i("sdio init complete");
}

static void my_spi_init() {
  // clock
  rcu_periph_clock_enable(RCU_GPIOB);
  rcu_periph_clock_enable(RCU_SPI1);
  rcu_periph_clock_enable(RCU_DMA0);
  // io
  gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_13 | GPIO_PIN_15);
  gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                          GPIO_PIN_13 | GPIO_PIN_15);
  gpio_af_set(GPIOB, GPIO_AF_5, GPIO_PIN_13 | GPIO_PIN_15);

  // SPI1
  spi_parameter_struct spi_struct = {
      .device_mode          = SPI_MASTER,
      .trans_mode           = SPI_TRANSMODE_BDTRANSMIT,
      .frame_size           = SPI_FRAMESIZE_8BIT,
      .nss                  = SPI_NSS_SOFT,
      .endian               = SPI_ENDIAN_MSB,
      .clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE,
      .prescale             = SPI_PSC_2,
  };
  spi_init(SPI1, &spi_struct);
  spi_enable(SPI1);

  // dma
  spi_dma_enable(SPI1, SPI_DMA_TRANSMIT);

  nvic_irq_enable(DMA0_Channel4_IRQn, 4, 0);

  log_i("spi init complete");

  // 128 * 160 * 2 = 40960 Byte 一帧
  //  10 Mhz SPI
  //  24 fps/s = 41.67 ms/fps
  //  40960 * 8 / 10000000 = 0.032768 s = 32.768 ms
}

static void my_dci_init() {
  // clock
  rcu_periph_clock_enable(RCU_DCI);
  rcu_periph_clock_enable(RCU_DMA1);
  rcu_periph_clock_enable(RCU_GPIOA);
  rcu_periph_clock_enable(RCU_GPIOB);
  rcu_periph_clock_enable(RCU_GPIOC);
  rcu_periph_clock_enable(RCU_GPIOE);
  // io
  // DCI_HSYNC
  gpio_af_set(GPIOA, GPIO_AF_13, GPIO_PIN_4);
  gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_4);
  gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4);
  // DCI_VSYNC
  gpio_af_set(GPIOB, GPIO_AF_13, GPIO_PIN_7);
  gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_7);
  gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7);
  // DCI_PIXCLK
  gpio_af_set(GPIOA, GPIO_AF_13, GPIO_PIN_6);
  gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_6);
  gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6);
  // DCI_D0
  gpio_af_set(GPIOC, GPIO_AF_13, GPIO_PIN_6);
  gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_6);
  gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6);
  // DCI_D1
  gpio_af_set(GPIOC, GPIO_AF_13, GPIO_PIN_7);
  gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_7);
  gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7);
  // DCI_D2
  gpio_af_set(GPIOE, GPIO_AF_13, GPIO_PIN_0);
  gpio_mode_set(GPIOE, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_0);
  gpio_output_options_set(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0);
  // DCI_D3
  gpio_af_set(GPIOE, GPIO_AF_13, GPIO_PIN_1);
  gpio_mode_set(GPIOE, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_1);
  gpio_output_options_set(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1);
  // DCI_D4
  gpio_af_set(GPIOE, GPIO_AF_13, GPIO_PIN_4);
  gpio_mode_set(GPIOE, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_4);
  gpio_output_options_set(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4);
  // DCI_D5
  gpio_af_set(GPIOB, GPIO_AF_13, GPIO_PIN_6);
  gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_6);
  gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6);
  // DCI_D6
  gpio_af_set(GPIOB, GPIO_AF_13, GPIO_PIN_8);
  gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_8);
  gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
  // DCI_D7
  gpio_af_set(GPIOB, GPIO_AF_13, GPIO_PIN_9);
  gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_9);
  gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);

  // dci
  dci_parameter_struct dci_struct = {
      .capture_mode     = DCI_CAPTURE_MODE_SNAPSHOT,
      .clock_polarity   = DCI_CK_POLARITY_RISING,
      .hsync_polarity   = DCI_HSYNC_POLARITY_LOW,
      .vsync_polarity   = DCI_VSYNC_POLARITY_HIGH,
      .frame_rate       = DCI_FRAME_RATE_ALL,
      .interface_format = DCI_INTERFACE_FORMAT_8BITS,
  };
  dci_init(&dci_struct);

  // dma

  dma_channel_subperipheral_select(DMA1, DMA_CH7, DMA_SUBPERI1);

  dma_interrupt_enable(DMA1, DMA_CH7, DMA_INT_FTF);
  nvic_irq_enable(DMA1_Channel7_IRQn, 4, 0);

  log_i("dci init complete");
}

static void i2c2_init() {
  // clock
  rcu_periph_clock_enable(RCU_GPIOA);
  rcu_periph_clock_enable(RCU_GPIOB);
  rcu_periph_clock_enable(RCU_I2C2);
  // i2c restore
  gpio_mode_set(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_8);
  gpio_mode_set(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_4);

  if (gpio_input_bit_get(GPIOA, GPIO_PIN_8) == RESET ||
      gpio_input_bit_get(GPIOB, GPIO_PIN_4) == RESET) {
    gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_8);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                            GPIO_PIN_8);
    for (size_t i = 0; i < 9; i++) {
      gpio_bit_reset(GPIOA, GPIO_PIN_8);
      vTaskDelay(pdMS_TO_TICKS(1));
      gpio_bit_set(GPIOA, GPIO_PIN_8);
      vTaskDelay(pdMS_TO_TICKS(1));
    }
  }
  // io
  gpio_af_set(GPIOA, GPIO_AF_4, GPIO_PIN_8);
  gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_8);
  gpio_output_options_set(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_8);

  gpio_af_set(GPIOB, GPIO_AF_9, GPIO_PIN_4);
  gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_4);
  gpio_output_options_set(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_4);
  // i2c
  i2c_clock_config(I2C2, 400000, I2C_DTCY_16_9);
  i2c_mode_addr_config(I2C2, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, 0x72);
  i2c_enable(I2C2);
  i2c_ack_config(I2C2, I2C_ACK_ENABLE);

  log_i("i2c2 init complete");
}

static void i2c_init() {
  i2c2_init();

  log_i("i2c init complete");
}

static void my_timer_init() {
  // clock
  rcu_periph_clock_enable(RCU_GPIOA);
  rcu_periph_clock_enable(RCU_TIMER1);
  // io
  // timer1
  // 蜂鸣器
  gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_15);
  gpio_af_set(GPIOA, GPIO_AF_1, GPIO_PIN_15);

  timer_parameter_struct timer_init_param;
  timer_oc_parameter_struct timer_oc_init_param;

  // timer1
  timer_init_param.prescaler         = SystemCoreClock / 1U / 1000000U - 1U;
  timer_init_param.alignedmode       = TIMER_COUNTER_EDGE;
  timer_init_param.counterdirection  = TIMER_COUNTER_UP;
  timer_init_param.clockdivision     = TIMER_CKDIV_DIV4;
  timer_init_param.period            = 1000000U / 4000U - 1U;
  timer_init_param.repetitioncounter = 0;
  timer_init(TIMER1, &timer_init_param);
  timer_oc_init_param.outputstate  = TIMER_CCX_ENABLE;
  timer_oc_init_param.outputnstate = TIMER_CCXN_DISABLE;
  timer_oc_init_param.ocpolarity   = TIMER_OC_POLARITY_HIGH;
  timer_oc_init_param.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
  timer_oc_init_param.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
  timer_oc_init_param.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;
  timer_channel_output_config(TIMER1, TIMER_CH_0, &timer_oc_init_param);
  timer_channel_output_mode_config(TIMER1, TIMER_CH_0, TIMER_OC_MODE_PWM0);
  timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_0, 0);
  timer_enable(TIMER1);

  log_i("timer init complete");
}

static void adc_init() {
  // clock
  rcu_periph_clock_enable(RCU_ADC0);
  // io
  gpio_mode_set(GPIOB, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_0);
  // adc0
  adc_resolution_config(ADC0, ADC_RESOLUTION_12B);  // 设置 adc 精度
  adc_data_alignment_config(ADC0,
                            ADC_DATAALIGN_RIGHT);  // 设置 adc 数据对齐方式

  // 配置规则组长度
  adc_channel_length_config(ADC0, ADC_ROUTINE_CHANNEL, 1);

  // 配置规则组
  adc_routine_channel_config(ADC0, 0, ADC_CHANNEL_8, ADC_SAMPLETIME_480);

  // adc 模式
  adc_special_function_config(ADC0, ADC_SCAN_MODE, ENABLE);

  adc_end_of_conversion_config(ADC0, ADC_EOC_SET_SEQUENCE);

  adc_enable(ADC0);
  vTaskDelay(pdMS_TO_TICKS(1));
  adc_calibration_enable(ADC0);

#warning ""
}

static void on_chip_hw_init() {
  gpio_init();
  adc_init();
  sdio_init();
  my_spi_init();
  my_dci_init();
  i2c_init();
  my_timer_init();
  log_i("on chip hw init complete");
}

static void mlx90640_init() {
  int32_t ret;

  // 申请内存
  uint16_t* p_mlx90640_eeprom_data = pvPortMalloc(832 * 2);
  configASSERT(p_mlx90640_eeprom_data);

  // 设置刷新率
  MLX90640_SetRefreshRate(0x33 << 1, 3);

  // 读取MLX90640校正数据
  ret = MLX90640_DumpEE(0x33 << 1, p_mlx90640_eeprom_data);

  log_i("mlx90640 dumpee retun code: %d", ret);
  if (ret != MLX90640_NO_ERROR) {
    log_e("mlx90640 dumpee fail, init fail");
    return;
  }

  // 解析校正数据
  ret = MLX90640_ExtractParameters(p_mlx90640_eeprom_data, &mlx90640_params);

  if (ret == MLX90640_NO_ERROR)
    log_i("mlx90640 exparams retun code: %d", ret);
  else {
    log_w(
        "mlx90640 eeprom data is corrupted, and the repair data will be "
        "loaded. This can make the sensor work normally, but it may make the "
        "data inaccurate.");
    // 解析失败，证明传感器校正数据损坏。在此会加载正常传感器的校正数据，可以使传感器继续运行，但可能会使传感器准确性下降。
    memcpy(p_mlx90640_eeprom_data, mlx90640_eeprom_repair_data,
           sizeof(mlx90640_eeprom_repair_data));
    // 重新解析
    ret = MLX90640_ExtractParameters(p_mlx90640_eeprom_data, &mlx90640_params);
    log_i("mlx90640 ext params retun code: %d", ret);
    if (ret != MLX90640_NO_ERROR) {
      log_a("mlx90640 is corrupted.");
      return;
    }
  }

  // 释放内存
  vPortFree(p_mlx90640_eeprom_data);
}

static void my_ov7725_init() { ov7725_init(); }

static void on_board_hw_init() {
  LCD_Init();
  mlx90640_init();
  my_ov7725_init();
  log_i("on board hw init complete");
}

void hw_init() {
  on_chip_hw_init();
  on_board_hw_init();

  log_i("hw init complete");
}
