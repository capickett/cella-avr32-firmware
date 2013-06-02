/*
 * security.c
 *
 * Created: 5/19/2013 7:22:31 PM
 *  Author: Matt Dorsett
 */ 

#include <string.h>
#include "sd_access.h"
#include "sd_mmc.h"
#include "aes_dma.h"
#include "ctrl_access.h"
#include "security.h"
#include "polarssl/sha2.h"
#include "udc.h"
#include "delay.h"

bool data_mounted;
bool data_locked;
static uint8_t src_buf[SD_MMC_BLOCK_SIZE * SD_BLOCKS_PER_ACCESS];
static uint8_t dest_buf[SD_MMC_BLOCK_SIZE * SD_BLOCKS_PER_ACCESS];
static uint8_t old_hash_cipher_key[HASH_LENGTH];
static uint8_t new_hash_cipher_key[HASH_LENGTH];

void sd_access_mount_data()
{
	data_mounted = true;
}

void sd_access_unmount_data()
{
	data_mounted = false;
}

void sd_access_lock_data()
{
	data_locked = true;
}

void sd_access_unlock_data()
{
	data_locked = false;
}

void sd_access_init()
{
	data_mounted = false;
	data_locked = true;
}

bool sd_access_unlock_drive(uint8_t* passwd) {
	if (!data_locked) {		
		sd_access_mount_data();
		return true;
	} else {
		return false;
	}
}

bool sd_access_factory_reset()
{
	return true;
}

/* Changes the encryption level of the drive.
   If encrypt is true: encrypt drive, else decrypt */
uint8_t sd_change_encryption(uint8_t slot, bool encrypt, bool change_key, uint8_t *old_passwd, uint8_t *new_passwd)
{
	sd_mmc_err_t err;
	uint32_t i, nb_blocks;
	encrypt_config_t *config_ptr = NULL;
	
	security_get_user_config(&config_ptr);
	if ((encrypt == config_ptr->encryption_level) && !change_key)
		return CTRL_GOOD;
	
	if (change_key) {
		sha2(old_passwd, MAX_PASS_LENGTH, old_hash_cipher_key, 0);
		sha2(new_passwd, MAX_PASS_LENGTH, new_hash_cipher_key, 0);
	}
	
	if (old_hash_cipher_key == new_hash_cipher_key)
		return CTRL_GOOD;
	
	do {
		err = sd_mmc_check(slot);
		if ((SD_MMC_ERR_NO_CARD != err)
		&& (SD_MMC_INIT_ONGOING != err)
		&& (SD_MMC_OK != err)) {
			while (SD_MMC_ERR_NO_CARD != sd_mmc_check(slot)) {
			}
		}
	} while (SD_MMC_OK != err);
	
	nb_blocks = sd_mmc_get_capacity(slot) * (1024 / SD_MMC_BLOCK_SIZE);
	
	for (i = 0; i < nb_blocks / SD_BLOCKS_PER_ACCESS; ++i) {
		if (SD_MMC_OK != sd_mmc_init_read_blocks(slot, i, SD_BLOCKS_PER_ACCESS))
			return CTRL_FAIL;
		if (SD_MMC_OK != sd_mmc_start_read_blocks(src_buf, SD_BLOCKS_PER_ACCESS))
			return CTRL_FAIL;
		if (SD_MMC_OK != sd_mmc_wait_end_of_read_blocks())
			return CTRL_FAIL;
		aes_set_key(&AVR32_AES, (unsigned int *)old_hash_cipher_key);
		ram_aes_ram(change_key ? false : encrypt, SD_MMC_BLOCK_SIZE * SD_BLOCKS_PER_ACCESS / sizeof(unsigned int), (unsigned int *)src_buf, (unsigned int *)dest_buf);
		if (change_key) {
			aes_set_key(&AVR32_AES, (unsigned int *)new_hash_cipher_key);
			ram_aes_ram(true, SD_MMC_BLOCK_SIZE * SD_BLOCKS_PER_ACCESS / sizeof(unsigned int), (unsigned int *)dest_buf, (unsigned int *)src_buf);
		}
		if (SD_MMC_OK != sd_mmc_init_write_blocks(slot, i, SD_BLOCKS_PER_ACCESS))
			return CTRL_FAIL;
		if (SD_MMC_OK != sd_mmc_start_write_blocks(src_buf, SD_BLOCKS_PER_ACCESS))
			return CTRL_FAIL;
		if (SD_MMC_OK != sd_mmc_wait_end_of_write_blocks())
			return CTRL_FAIL;
	}	
	return CTRL_GOOD;
}