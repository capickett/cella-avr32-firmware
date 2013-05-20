/*
 * data_mount.h
 *
 * Created: 5/19/2013 7:19:58 PM
 *  Author: administrator
 */ 

#include "compiler.h"

#ifndef DATA_MOUNT_H_
#define DATA_MOUNT_H_

#define SD_LUN_INDEX 1

extern bool data_mounted;

void mount_data(void);

void unmount_data(void);

#endif /* DATA_MOUNT_H_ */