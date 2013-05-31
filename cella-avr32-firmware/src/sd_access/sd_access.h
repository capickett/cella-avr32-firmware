/*
 * data_mount.h
 *
 * Created: 5/19/2013 7:19:58 PM
 *  Author: administrator
 */ 

#include "compiler.h"
#include "conf_access.h"

#ifndef DATA_MOUNT_H_
#define DATA_MOUNT_H_

#define SD_LUN_INDEX			LUN_ID_SD_MMC_0_MEM
#define SD_BLOCKS_PER_ACCESS	16

extern bool data_mounted;
extern bool data_locked;

void sd_access_init(void);

void sd_access_mount_data(void);

void sd_access_unmount_data(void);

void sd_access_lock_data(void);

void sd_access_unlock_data(void);

bool sd_access_unlock_drive(uint8_t* passwd);

uint8_t sd_change_encryption(uint8_t slot, bool encrypt, bool change_key, uint8_t *old_passwd, uint8_t *new_passwd);

#endif /* DATA_MOUNT_H_ */