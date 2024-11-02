#ifndef _BUZZER_H_
#define _BUZZER_H_

typedef enum {
  BuzzerToneSuccess,
  BuzzerToneFail,
  BuzzerToneWarning,
  BuzzerToneClick,
  BuzzerTone4khz500ms,
  BuzzerToneMax,
} BuzzerToneNum_t;

/**
 * @brief 蜂鸣器相关初始化
 *
 */
void buzzer_init();

/**
 * @brief 蜂鸣器发声
 *
 * @param tone_num 音频编号
 */
void buzzer_sing(BuzzerToneNum_t tone_num);

#endif