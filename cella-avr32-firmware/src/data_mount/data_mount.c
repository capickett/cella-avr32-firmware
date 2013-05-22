/*
 * security.c
 *
 * Created: 5/19/2013 7:22:31 PM
 *  Author: Matt Dorsett
 */ 

#include "data_mount.h"

bool data_mounted = false;

void mount_data()
{
	data_mounted = true;
}

void unmount_data()
{
	data_mounted = false;
}