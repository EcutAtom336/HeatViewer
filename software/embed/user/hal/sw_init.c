#include "sw_init.h"
///
#define LOG_TAG "sw init"
#include "elog.h"
///
#include "buzzer.h"
#include "multi_button_port.h"
#include "my_lcd.h"
#include "sd_card_ex.h"
#include "on_board_led.h"
///
#include "sd_card.h"
///
#include "ff.h"
//
#include <stdio.h>

static void my_elog_init() {
  elog_init();
  elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
  elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
  elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
  elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
  elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
  elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
  elog_start();
}

static uint8_t fatfs_buf[1024];
static FATFS fatfs;
static FIL file;

static void fatfs_init() {
  FRESULT fatfs_ret;

  if (sd_card_get_phy_status() == SdCardNotExist) {
    log_i("sd card not found.");
    return;
  }

  fatfs_ret = f_mount(&fatfs, "0:", 1);

  if (fatfs_ret != FR_OK) {
    log_w("f_mount fail, code: %d", fatfs_ret);
  } else if (fatfs_ret == FR_NO_FILESYSTEM) {
    log_w("f_mount fail, reason: no file system, sd card will be formatted");

    fatfs_ret = f_mkfs("U", FM_FAT32, 1024, fatfs_buf, sizeof((fatfs_buf)));
    if (fatfs_ret != FR_OK) {
      log_w("f_mkfs fail, code: %d", fatfs_ret);
    } else {
      log_w("sd card formatted success");
    }
  } else {
    log_i("sd mount success");
  }
}

void sw_init() {
  // my_elog_init();
  lcd_handler_init();
  fatfs_init();
  multi_button_port_init();
  power_manage_init();
  buzzer_init();
  on_board_led_init();  // 开发板 led 初始化
}
