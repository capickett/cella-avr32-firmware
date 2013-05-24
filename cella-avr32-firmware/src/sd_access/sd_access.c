/*
 * security.c
 *
 * Created: 5/19/2013 7:22:31 PM
 *  Author: Matt Dorsett
 */ 

#include "sd_access.h"
#include "sd_mmc.h"
#include "aes_dma.h"
#include "ctrl_access.h"
#include "security.h"

bool data_mounted = true;
static uint8_t src_buf[SD_MMC_BLOCK_SIZE];
static uint8_t dest_buf[SD_MMC_BLOCK_SIZE];

void mount_data()
{
	data_mounted = true;
}

void unmount_data()
{
	data_mounted = false;
}

/* Changes the encryption level of the drive.
   If encrypt is true: encrypt drive, else decrypt */
uint8_t sd_change_encryption(uint8_t slot, bool encrypt)
{
	if (encrypt != user_data->config.encryption_level)
		return CTRL_GOOD;
	
	uint32_t nb_blocks = sd_mmc_get_capacity(SD_SLOT_INDEX) * 2;	
	//uint32_t nb_blocks = 7822336 * 2;
	
	for (uint32_t i = 0; i < nb_blocks; ++i) {
		switch (sd_mmc_init_read_blocks(slot, i, 1)) {
			case SD_MMC_OK:
			break;
			case SD_MMC_ERR_NO_CARD:
			return CTRL_NO_PRESENT;
			default:
			return CTRL_FAIL;
		}
		if (SD_MMC_OK != sd_mmc_start_read_blocks(src_buf, 1)) {
			return CTRL_FAIL;
		}
		if (SD_MMC_OK != sd_mmc_wait_end_of_read_blocks()) {
			return CTRL_FAIL;
		}
		ram_aes_ram(encrypt, SD_MMC_BLOCK_SIZE/sizeof(unsigned int), (unsigned int *)src_buf, (unsigned int *)dest_buf);
		switch (sd_mmc_init_write_blocks(slot, i, 1)) {
			case SD_MMC_OK:
			break;
			case SD_MMC_ERR_NO_CARD:
			return CTRL_NO_PRESENT;
			default:
			return CTRL_FAIL;
		}
		if (SD_MMC_OK != sd_mmc_start_write_blocks(dest_buf, 1)) {
			return CTRL_FAIL;
		}
		if (SD_MMC_OK != sd_mmc_wait_end_of_write_blocks()) {
			return CTRL_FAIL;
		}
	}
	return CTRL_GOOD;
}