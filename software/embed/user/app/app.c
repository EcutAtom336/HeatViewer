#include "app.h"
///
#define LOG_TAG "app"
#include "elog.h"
///
#include "buzzer.h"
#include "laser.h"
#include "led.h"
#include "color.h"
#include "multi_button_port.h"
#include "my_lcd.h"
#include "util_image.h"
#include "mlx90640_handler.h"
///
#include "ov7725.h"
///
#include "multi_button.h"
///
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
///
#include "gd32f4xx.h"
///
#include <stdio.h>

static TaskHandle_t __attribute__((section(".DTCM"))) app_task_handle;
static StaticTask_t __attribute__((section(".DTCM"))) app_task;
static uint16_t app_task_stack[2048];

static QueueHandle_t __attribute__((section(".DTCM"))) app_btn_evt_queue_handle;
static StaticQueue_t __attribute__((section(".DTCM"))) app_btn_evt_queue;
static uint8_t app_queue_buf[8];

static Button __attribute__((section(".DTCM"))) btn_menu, btn_up, btn_down,
    btn_trigger;

static float_t __attribute__((section(".DTCM"))) emissivity     = 0.95F;
static int16_t __attribute__((section(".DTCM"))) temp_alarm_val = 40;
static uint8_t __attribute__((section(".DTCM"))) menu_idx = 0, is_select = 0,
                                                 show_hv         = 1,
                                                 temp_alarm_mode = 0;

static uint16_t frame0[160 * 128];
static uint16_t __attribute__((section(".DTCM"))) frame1[160 * 128];

const uint16_t SELECT_INDICATER_POS[3][2] = {
    {0, 15},
    {0, 30},
    {0, 45},
};

static void app_code() {
  TickType_t tick;

  uint8_t btn_evt;

  BaseType_t freertos_ret = pdFALSE;

  float_t l_temp, c_temp, h_temp;
  uint32_t h_temp_idx, l_temp_idx;

  uint8_t tmp_str[16];

  // 配置 DCI DMA
  dma_single_data_parameter_struct init_struct = {
      .periph_addr         = DCI + 0x28U,
      .periph_inc          = DMA_PERIPH_INCREASE_DISABLE,
      .memory0_addr        = (uint32_t)frame0,
      .memory_inc          = DMA_MEMORY_INCREASE_ENABLE,
      .periph_memory_width = DMA_PERIPH_WIDTH_32BIT,
      .circular_mode       = DMA_CIRCULAR_MODE_DISABLE,
      .direction           = DMA_PERIPH_TO_MEMORY,
      .number              = 160 * 128 / 2,
      .priority            = DMA_PRIORITY_HIGH,
  };
  dma_single_data_mode_init(DMA1, DMA_CH7, &init_struct);

  // 使能 DCI
  dci_enable();
  dci_crop_window_config((320 - 160 + 80) / 2, (240 - 128 - 38) / 2,
                         160 * 2 - 1, 128 * 2 - 1);
  dci_crop_window_enable();

  while (1) {
    // 接收按键事件
    freertos_ret = xQueueReceive(app_btn_evt_queue_handle, &btn_evt, 0);
    if (freertos_ret == pdTRUE) {
      if (btn_evt == 0) {
        if (util_image_save_rgb565_as_bmp(lcd_buf))
          buzzer_sing(BuzzerToneFail);
        else
          buzzer_sing(BuzzerToneSuccess);
      } else if (btn_evt == 1) {
        mlx90640_handler_set_emissivity(emissivity);
      }
    }

    if (show_hv) {
      // 配置 DCI 读取 OV7725 数据
      dma_single_data_mode_init(DMA1, DMA_CH7, &init_struct);
      dma_channel_enable(DMA1, DMA_CH7);
      dci_capture_enable();

      // 等待 OV7725 数据读取完成
      ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

      // 对可见光数据进行缩放，以对齐热成像数据
      util_image_rgb565_zoom(frame0, frame1, 0.7f, 24 - 10, 20);

      memset(frame0, 0, sizeof(frame0));

      // 边缘检测
      util_image_rgb565grey_part_edge_detection(frame1, frame0, 16, 20, 112,
                                                89);

      // 根据热成像传感器数据对经过边缘检测后的可见光图像进行上色
      util_image_colour(frame0, merge_result, &h_temp, &h_temp_idx, &c_temp,
                        &l_temp, &l_temp_idx);

      // 显示结果
      lcd_show_rgb565_pic(0, 0, 128, 160, frame0);

      // 显示中心准星和高低温准星
      lcd_show_sight(79, 63);
      lcd_show_sight((31 - h_temp_idx % 32) * 5, h_temp_idx / 32 * 5);
      lcd_show_sight((31 - l_temp_idx % 32) * 5, l_temp_idx / 32 * 5);

      // 显示高低温度和中心温度
      if (l_temp < 0.0F || l_temp >= 100.0F)
        sprintf((char *)tmp_str, "%.1f", l_temp);
      else
        sprintf((char *)tmp_str, "%.2f", l_temp);
      lcd_show_str(0, 116, tmp_str, RGB565_WHITE, RGB565_BLACK, 12);

      if (c_temp < 0.0F || c_temp >= 100.0F)
        sprintf((char *)tmp_str, "%.1f", c_temp);
      else
        sprintf((char *)tmp_str, "%.2f", c_temp);
      lcd_show_str(70, 105, tmp_str, RGB565_WHITE, RGB565_BLACK, 12);

      if (h_temp < 0.0F || h_temp >= 100.0F)
        sprintf((char *)tmp_str, "%.1f", h_temp);
      else
        sprintf((char *)tmp_str, "%.2f", h_temp);
      lcd_show_str(131, 116, tmp_str, RGB565_WHITE, RGB565_BLACK, 12);

      // 显示发射率
      sprintf((char *)tmp_str, "%.2f", emissivity);
      lcd_show_str(135, 0, tmp_str, RGB565_WHITE, RGB565_WHITE, 12);

      // 显示色带
      lcd_show_rainbow(30, 117);

      // 温度报警
      if ((temp_alarm_mode == 1 && l_temp < temp_alarm_val) ||
          (temp_alarm_mode == 2 && h_temp > temp_alarm_val)) {
        // 低温报警模式
        buzzer_sing(BuzzerTone4khz500ms);
      }
    } else {
      lcd_fill(0, 0, 159, 127, RGB565_BLACK);

      lcd_show_font(12, 15,
                    (uint8_t *)(temp_alarm_mode ? (temp_alarm_mode == 1
                                                       ? "温度报警：低温报警"
                                                       : "温度报警：高温报警")
                                                : "温度报警：关"),
                    RGB565_WHITE, RGB565_WHITE, FontSize12);

      lcd_show_font(12, 30, (uint8_t *)"温度报警值：", RGB565_WHITE,
                    RGB565_WHITE, FontSize12);
      lcd_show_num(80, 30, temp_alarm_val, 3, RGB565_WHITE, RGB565_WHITE, 12);

      char emssivity_str[5];
      lcd_show_font(12, 45, (uint8_t *)"发射率：", RGB565_WHITE, RGB565_WHITE,
                    FontSize12);
      sprintf(emssivity_str, "%.2f", emissivity);
      lcd_show_str(70, 45, (uint8_t *)emssivity_str, RGB565_WHITE, RGB565_WHITE,
                   12);

      lcd_show_str(
          SELECT_INDICATER_POS[menu_idx][0], SELECT_INDICATER_POS[menu_idx][1],
          (uint8_t *)(is_select ? ">>" : ">"), RGB565_WHITE, RGB565_WHITE, 12);
    }
    // 显示电池图标
    lcd_show_battery(1, 1, power_manage_get_battery_status());

    // 请求刷新 LCD
    lcd_refreash_request();

    vTaskDelay(pdMS_TO_TICKS(show_hv ? 500 : 50));
  }
}

static void btn_handler(void *param) {
  struct Button *btn = (struct Button *)param;
  PressEvent evt     = get_button_event(btn);

  if (evt == PRESS_DOWN) buzzer_sing(BuzzerToneClick);

  switch (btn->button_id) {
    case BtnMenu: {
      if (evt == PRESS_DOWN) {
        uint8_t v = 1;
        xQueueSend(app_btn_evt_queue_handle, &v, 0);
        show_hv = !show_hv;
      }
      break;
    }
    case BtnUp: {
      if (evt == PRESS_DOWN) {
        if (show_hv) {
          led_toggle();
        } else {
          if (is_select) {
            switch (menu_idx) {
              case 0:
                if (temp_alarm_mode < 2)
                  temp_alarm_mode++;
                else
                  temp_alarm_mode = 0;
                break;
              case 1:
                if (temp_alarm_val < 200U) { temp_alarm_val += 1U; }
                break;
              case 2:
                if (emissivity < 0.95F)
                  emissivity += 0.01F;
                else if (emissivity > 0.95F)
                  emissivity = 0.95F;
                break;
            }
          } else {
            if (menu_idx != 0) menu_idx--;
          }
        }
      } else if (evt == LONG_PRESS_HOLD) {
        switch (menu_idx) {
          case 1:
            if (temp_alarm_val < 200U) { temp_alarm_val += 1U; }
            break;
          case 2:
            if (emissivity < 0.95F)
              emissivity += 0.01F;
            else if (emissivity > 0.95F)
              emissivity = 0.95F;
            break;
          default:
            break;
        }
      }
      break;
    }
    case BtnDown: {
      if (evt == PRESS_DOWN) {
        if (show_hv) {
          laser_toggle();
        } else {
          if (is_select) {
            switch (menu_idx) {
              case 0:
                if (temp_alarm_mode != 0)
                  temp_alarm_mode--;
                else
                  temp_alarm_mode = 2;
                break;
              case 1:
                if (temp_alarm_val > -20) { temp_alarm_val -= 1; }
                break;
              case 2:
                if (emissivity > 0.05F)
                  emissivity -= 0.01F;
                else if (emissivity < 0.05F)
                  emissivity = 0.05F;
                break;
            }
          } else {
            if (menu_idx < 2) menu_idx++;
          }
        }
      } else if (evt == LONG_PRESS_HOLD) {
        if (is_select) {
          switch (menu_idx) {
            case 1:
              if (temp_alarm_val > -20) { temp_alarm_val -= 1; }
              break;
            case 2:
              if (emissivity > 0.05F)
                emissivity -= 0.01F;
              else if (emissivity < 0.05F)
                emissivity = 0.05F;
              break;
            default:
              break;
          }
        }
      }
      break;
    }
    case BtnTrigger: {
      if (evt == PRESS_DOWN) {
        if (show_hv) {
          uint8_t v = 0;
          xQueueSend(app_btn_evt_queue_handle, &v, 0);
        } else {
          is_select = !is_select;
        }
      }
      break;
    }
  }
}

void app_init() {
  app_btn_evt_queue_handle =
      xQueueCreateStatic(4, 1, app_queue_buf, &app_btn_evt_queue);
  configASSERT(app_btn_evt_queue_handle);

  configASSERT(!(sizeof(app_task_stack) % 4));
  app_task_handle =
      xTaskCreateStatic(app_code, "app", sizeof(app_task_stack) / 4, NULL, 12,
                        (StackType_t *)app_task_stack, &app_task);
  configASSERT(app_task_handle);

  button_init(&btn_menu, multi_button_read_btn, 0, BtnMenu);
  button_init(&btn_up, multi_button_read_btn, 0, BtnUp);
  button_init(&btn_down, multi_button_read_btn, 0, BtnDown);
  button_init(&btn_trigger, multi_button_read_btn, 0, BtnTrigger);

  button_attach(&btn_menu, PRESS_DOWN, btn_handler);
  button_attach(&btn_up, PRESS_DOWN, btn_handler);
  button_attach(&btn_up, LONG_PRESS_HOLD, btn_handler);
  button_attach(&btn_down, PRESS_DOWN, btn_handler);
  button_attach(&btn_down, LONG_PRESS_HOLD, btn_handler);
  button_attach(&btn_trigger, PRESS_DOWN, btn_handler);

  button_start(&btn_menu);
  button_start(&btn_up);
  button_start(&btn_down);
  button_start(&btn_trigger);
}

void DMA1_Channel7_IRQHandler() {
  if (dma_flag_get(DMA1, DMA_CH7, DMA_FLAG_FTF)) {
    dma_flag_clear(DMA1, DMA_CH7, DMA_FLAG_FTF);
    xTaskNotifyFromISR(app_task_handle, 1, eIncrement, NULL);
  }
}
