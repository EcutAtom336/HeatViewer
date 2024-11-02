#include "mlx90640_handler.h"
#include <stdlib.h>
///
#define LOG_TAG "mlx90640_handler"
#include "elog.h"
///
#include "mlx90640_user.h"
///
#include "MLX90640_API.h"
///
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
///
#include <math.h>
#include <stdint.h>

const uint8_t MLX90640_ADDRESS = 0x33 << 1;

static TaskHandle_t
    __attribute__((section(".DTCM"))) mlx90640_handler_task_handle;
static StaticTask_t __attribute__((section(".DTCM"))) mlx90640_handler_task;
static uint8_t mlx90640_handler_task_stack[2048];

static StaticQueue_t __attribute__((section(".DTCM"))) mlx90640_handler_queue;
static QueueHandle_t
    __attribute__((section(".DTCM"))) mlx90640_handler_queue_handle;
static uint8_t __attribute__((
    section(".DTCM"))) mlx90640_handler_queue_buf[sizeof(float_t) * 2];

static uint16_t frame_raw[834];

static float_t subpage_result[32 * 24];
float_t merge_result[32 * 24];

static uint8_t __attribute__((section(".DTCM"))) frame_result_is_available;

void mlx90640_handler() {
  int32_t ret;
  int subpage_num;
  float_t emissivity = 0.95f, tr;
  TickType_t tick;
  uint8_t is_sync;
  BaseType_t freertos_ret;

  while (1) {
    // 等待信号
    // ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000));

    if (!is_sync) {
      ret = MLX90640_SynchFrame(MLX90640_ADDRESS);
      if (ret) {
        log_w("sync fail, code: %d", ret);
        continue;
      }
      is_sync = 1;
    }

    // 从 mlx90640 读取数据
    ret = MLX90640_GetFrameData(MLX90640_ADDRESS, frame_raw);
    if (ret < 0) {
      log_w("read mlx90640 frame data fail, code: %d", ret);
      is_sync = 0;
      continue;
    }
    subpage_num = ret;
    tick        = xTaskGetTickCount();

    // 计算数据
    tr = MLX90640_GetTa(frame_raw, &mlx90640_params) - 8;
    MLX90640_CalculateTo(frame_raw, &mlx90640_params, emissivity, tr,
                         subpage_result);

    MLX90640_BadPixelsCorrection((&mlx90640_params)->brokenPixels,
                                 subpage_result, 1, &mlx90640_params);
    MLX90640_BadPixelsCorrection((&mlx90640_params)->outlierPixels,
                                 subpage_result, 1, &mlx90640_params);

    // 合并数据
    for (size_t i = subpage_num; i < 32 * 24; i += 2)
      merge_result[i] = subpage_result[i];

    if (frame_result_is_available != 2) { frame_result_is_available++; }

    xTaskDelayUntil(&tick, pdMS_TO_TICKS(225));

    freertos_ret = xQueueReceive(mlx90640_handler_queue_handle, &emissivity, 0);
  }
}

void mlx90640_handler_set_emissivity(float_t emissivity) {
  xQueueSend(mlx90640_handler_queue_handle, &emissivity, 0);
}

void mlx90640_handler_init() {
  mlx90640_handler_queue_handle = xQueueCreateStatic(
      2, sizeof(float_t), mlx90640_handler_queue_buf, &mlx90640_handler_queue);
  configASSERT(mlx90640_handler_queue_handle);

  configASSERT(!(sizeof(mlx90640_handler_task_stack) % 4));
  mlx90640_handler_task_handle = xTaskCreateStatic(
      mlx90640_handler, "mlx90640Handler",
      sizeof(mlx90640_handler_task_stack) / 4, NULL, configMAX_PRIORITIES - 2,
      (StackType_t *)mlx90640_handler_task_stack, &mlx90640_handler_task);
  configASSERT(mlx90640_handler_task_handle);
}
