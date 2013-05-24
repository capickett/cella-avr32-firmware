/*
 * data_mount.h
 *
 * Created: 5/19/2013 7:19:58 PM
 *  Author: administrator
 */ 

#include "compiler.h"

#ifndef DATA_MOUNT_H_
#define DATA_MOUNT_H_

#define SD_SLOT_INDEX 0
#define SD_LUN_INDEX (SD_SLOT_INDEX + 1)

extern bool data_mounted;

void mount_data(void);

void unmount_data(void);

uint8_t sd_change_encryption(uint8_t slot, bool encrypt);

#endif /* DATA_MOUNT_H_ */