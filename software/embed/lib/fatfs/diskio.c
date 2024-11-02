/*!
    \file    diskio.c
    \brief   Low level disk interface driver

    \version 2021-05-20, V1.0.0, firmware for GD32 USBFS&USBHS
*/

/*
    Copyright (c) 2021, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its
contributors may be used to endorse or promote products derived from this
software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* Includes ------------------------------------------------------------------*/
#include "diskio.h"
///
#define LOG_TAG "diskio"
#include "elog.h"
///
#include "sd_card_ex.h"
///
#include "sd_card.h"
///
#include "gd32f4xx.h"

#if defined(__GNUC__)
#ifndef __weak
#define __weak __attribute__((weak))
#endif
#endif

#define SD_CARD 0

sd_card_info_struct sd_card_info;

/*!
    \brief      Gets Disk Status
    \param[in]  pdrv: Physical drive number (0..)
    \param[out] none
    \retval     DSTATUS: Operation status
*/
DSTATUS disk_status(BYTE pdrv /* Physical drive number to identify the drive */
) {
  DSTATUS stat;

  switch (pdrv) {
    case SD_CARD: {
      stat = sd_card_get_phy_status() == SdCardExist ? RES_OK : RES_NOTRDY;
      break;
    }
    default: {
      log_a("unknow drv: %u", pdrv);
      break;
    }
  }

  return stat;
}

/*!
    \brief      Initializes a Drive
    \param[in]  pdrv: Physical drive number (0..)
    \param[out] none
    \retval     DSTATUS: Operation status
*/
DSTATUS disk_initialize(
    BYTE pdrv /* Physical drive nmuber to identify the drive */
) {
  DSTATUS stat = RES_OK;

  switch (pdrv) {
    case SD_CARD: {
      sd_error_enum sd_err;

      if (sd_card_get_phy_status() != SdCardExist) {
        log_i("sd card not found.");
        stat = RES_NOTRDY;
        break;
      }

      sd_err = sd_card_information_get(&sd_card_info);
      if (sd_err != SD_OK) {
        log_i("get sd card information fail, code: %d.", sd_err);
        stat = RES_NOTRDY;
        break;
      }

      sd_err = sd_card_select_deselect(sd_card_info.card_rca);
      if (sd_err != SD_OK) {
        log_w("deselectsd card fail, code: %d.", sd_err);
        stat = RES_NOTRDY;
        break;
      }

      uint32_t sd_card_state;
      sd_err = sd_cardstatus_get(&sd_card_state);
      if (sd_card_state & 0x02000000) {
        log_i("sd card is locked, try to unlock the sd card");
        sd_err = sd_lock_unlock(SD_UNLOCK);
        if (sd_err != SD_OK) {
          log_w("unlock fail, code: %d", sd_err);
          stat = RES_NOTRDY;
          break;
        } else
          log_i("unlock success");
      }

      sd_err = sd_transfer_mode_config(SD_POLLING_MODE);
      if (sd_err != SD_OK) {
        log_a("set sd card transfer mode fail, code: %dd", sd_err);
        stat = RES_ERROR;
      }

      stat = RES_OK;
      break;
    }
    default: {
      log_a("unknow drv: %u", pdrv);
      break;
    }
  }

  return stat;
}

/*!
    \brief      Reads Sector(s)
    \param[in]  pdrv: Physical drive number (0..)
    \param[in]  buff: Data buffer to store read data
    \param[in]  sector: Sector address (LBA)
    \param[in]  count: Number of sectors to write (1..128)
    \param[out] none
    \retval     DRESULT: Operation result
*/
DRESULT disk_read(BYTE pdrv,  /* Physical drive nmuber to identify the drive */
                  BYTE *buff, /* Data buffer to store read data */
                  DWORD sector, /* Sector address in LBA */
                  UINT count    /* Number of sectors to read */
) {
  DRESULT res;

  switch (pdrv) {
    case SD_CARD: {
      sd_error_enum sd_stat = SD_OK;

      if (sd_card_get_phy_status() != SdCardExist) {
        log_i("sd card not found.");
        res = RES_NOTRDY;
        break;
      }

      if (count > 1) {
        sd_stat = sd_multiblocks_read((uint32_t *)buff,
                                      sector * sd_card_info.card_blocksize,
                                      sd_card_info.card_blocksize, count);
        if (sd_stat != SD_OK)
          log_w("sd_multiblocks_read fail, code: %d", sd_stat);
      } else {
        sd_stat = sd_block_read((uint32_t *)buff,
                                sector * sd_card_info.card_blocksize,
                                sd_card_info.card_blocksize);
        if (sd_stat != SD_OK) log_w("sd_block_read fail, code: %d", sd_stat);
      }

      res = sd_stat == SD_OK ? RES_OK : RES_ERROR;

      break;
    }
    default: {
      log_a("unknow drv: %u", pdrv);
      break;
    }
  }

  return res;
}

/*!
    \brief      Writes Sector(s)
    \param[in]  pdrv: Physical drive number (0..)
    \param[in]  buff: Data to be written
    \param[in]  sector: Sector address (LBA)
    \param[in]  count: Number of sectors to write (1..128)
    \param[out] none
    \retval     DRESULT: Operation result
*/
#if _USE_WRITE == 1
DRESULT disk_write(BYTE pdrv, /* Physical drive nmuber to identify the drive */
                   const BYTE *buff, /* Data to be written */
                   DWORD sector,     /* Sector address in LBA */
                   UINT count        /* Number of sectors to write */
) {
  DRESULT res;

  switch (pdrv) {
    case SD_CARD: {
      sd_error_enum sd_stat = SD_OK;

      if (sd_card_get_phy_status() != SdCardExist) {
        log_i("sd card not found.");
        res = RES_NOTRDY;
        break;
      }

      if (count > 1) {
        sd_stat = sd_multiblocks_write((uint32_t *)buff,
                                       sector * sd_card_info.card_blocksize,
                                       sd_card_info.card_blocksize, count);
        if (sd_stat != SD_OK)
          log_w("sd_multiblocks_write fail, code: %d", sd_stat);
      } else {
        sd_stat = sd_block_write((uint32_t *)buff,
                                 sector * sd_card_info.card_blocksize,
                                 sd_card_info.card_blocksize);
        if (sd_stat != SD_OK) log_w("sd_block_write fail, code: %d", sd_stat);
      }

      res = sd_stat == SD_OK ? RES_OK : RES_ERROR;

      break;
    }
    default: {
      log_a("unknow drv: %u", pdrv);
      break;
    }
  }

  return res;
}
#endif /* _USE_WRITE == 1 */

/*!
    \brief      I/O control operation
    \param[in]  pdrv: Physical drive number (0..)
    \param[in]  cmd: Control code
    \param[in]  buff: Buffer to send/receive control data
    \param[out] none
    \retval     DRESULT: Operation result
*/
#if _USE_IOCTL == 1
DRESULT disk_ioctl(BYTE pdrv, /* Physical drive nmuber (0..) */
                   BYTE cmd,  /* Control code */
                   void *buff /* Buffer to send/receive control data */
) {
  DRESULT res;

  switch (pdrv) {
    case SD_CARD: {
      switch (cmd) {
        /*return sector number*/
        case GET_SECTOR_COUNT:
          *(DWORD *)buff =
              sd_card_info.card_capacity / (sd_card_info.card_blocksize);
          break;
        /*return each sector size*/
        case GET_SECTOR_SIZE:
          *(WORD *)buff = sd_card_info.card_blocksize;
          break;
        /*Returns the smallest unit of erased sector (unit 1)*/
        case GET_BLOCK_SIZE:
#define SD_CARD_BLOCK_SIZE 1
          *(DWORD *)buff = SD_CARD_BLOCK_SIZE;
#undef SD_CARD_BLOCK_SIZE
          break;
      }
      res = RES_OK;
      break;
    }
    default: {
      log_a("unknow drv: %u", pdrv);
      break;
    }
  }

  return res;
}
#endif /* _USE_IOCTL == 1 */

/*!
    \brief      Gets Time from RTC
    \param[in]  none
    \param[out] none
    \retval     Time in DWORD
*/
__weak DWORD get_fattime(void) { return 0; }
