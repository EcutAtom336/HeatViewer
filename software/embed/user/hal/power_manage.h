#ifndef _POWER_MANAGE_H_
#define _POWER_MANAGE_H_

typedef enum {
  BatteryLevelUrgent,
  BatteryLevelLow,
  BatteryLevelMid,
  BatteryLevelHigh,
  BatteryLevelCharge,
  BatteryLevelMax,
} BatteryLevel_t;

/**
 * @brief 电源管理初始化
 * 
 */
void power_manage_init();

/**
 * @brief 获取电池状态
 * 
 * @return BatteryLevel_t 电池状态枚举
 */
BatteryLevel_t power_manage_get_battery_status();

#endif  // !_POWER_MANAGE_H_