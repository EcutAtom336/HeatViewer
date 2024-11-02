#include "sd_card_ex.h"
///
#include "gd32f4xx.h"

SdCardPhyStatus_t sd_card_get_phy_status() {
  return gpio_input_bit_get(GPIOD, GPIO_PIN_3) == SET ? SdCardNotExist
                                                      : SdCardExist;
}
