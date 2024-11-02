#ifndef _SD_CARD_EX_H_
#define _SD_CARD_EX_H_

typedef enum  {
  SdCardNotExist,
  SdCardExist,
}SdCardPhyStatus_t;

SdCardPhyStatus_t sd_card_get_phy_status();

#endif  // !_SD_CARD_EX_H_