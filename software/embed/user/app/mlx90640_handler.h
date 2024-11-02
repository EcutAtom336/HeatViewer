#ifndef _MLX90640_HANDLER_H_

#include <math.h>

extern float_t merge_result[32 * 24];

/**
 * @brief MLX90640 数据处理任务初始化
 *
 */
void mlx90640_handler_init();

/**
 * @brief 设置发射率
 *
 * @param emissivity
 */
void mlx90640_handler_set_emissivity(float_t emissivity);

#endif  // !_MLX90640_HANDLER_H_
