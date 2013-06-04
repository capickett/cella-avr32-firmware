/*
 * msc_comm.c
 *
 * Created: 5/30/2013 2:44:24 PM
 *  Author: administrator
 */ 
#include <conf_access.h>
#include <fsaccess.h>
#include <fat.h>
#include <unistd.h>

#include "msc_comm.h"
#include "security.h"
#include "conf_security.h"
#include "sd_access.h"

static int fd;
static uint8_t password_buf[MAX_PASS_LENGTH];

void msc_comm_init(void) {
	

	//const uint8_t _MEM_TYPE_SLOW_ script_name[17] = 
	//	{'.', 'c', 'e', 'l', 'l', 'a', '_', 'u', 'n', 'l', 'o', 'c', 'k', '.', 's', 'h', 0};
	
	if (nav_drive_set(LUN_ID_VIRTUAL_MEM)) {
		nav_drive_format(FS_FORMAT_DEFAULT);
		nav_partition_mount();
		// TODO: get script into static array
		uint8_t _MEM_TYPE_SLOW_* script_buf;
		uint16_t script_size;
		//if (nav_file_create( (const FS_STRING) script_name ) && file_open(FOPEN_MODE_W)) {
		//	file_write_buf(script_buf, script_size);
		//}
		//file_close();
	}
}

bool file_exists(void) {
	char * file_name = "__cellaCMD";
	if ((fd = open(file_name, O_RDONLY)) >= 0)
		LED_On(LED2);
	else LED_Toggle(LED3);
}

static bool msc_comm_read_config(void)
{
	uint8_t config_string[sizeof(encrypt_config_t)];
	encrypt_config_t *config_ptr = NULL;
	security_get_config(&config_ptr);
	
	char encrypt_char;
	read(fd, &encrypt_char, 1);
	
	uint8_t encrypt_level = encrypt_char == '0' ? 0 : 1;
	
	if (encrypt_level > MAX_FACTOR - 1 || encrypt_level < MIN_FACTOR)
		return false;
	
	if (config_ptr->encryption_level != encrypt_level) {
		security_flash_write_config((encrypt_config_t *)config_string);
	}
	
	return true;
}

static void msc_comm_write_config(int res)
{
	encrypt_config_t *config_ptr = NULL;
	security_get_config(&config_ptr);
	
	char encrypt_char = config_ptr->encryption_level == 0 ? '0' : '1';
	
	write(res, &encrypt_char, 1);
}

static void msc_comm_write_ack(char ack, int res)
{
	write(res, &ack, 1);
}

void process_file(void) {
	int cmd = fd;
	int res = open(RESFILE, O_WRONLY);
	if ((cmd >= 0) && (res >= 0)) {
		uint8_t cmd_buf;
		read(cmd, &cmd_buf, 1);
		switch (cmd_buf) {
			case HANDLE_RESET:
				if (data_mounted) {
					msc_comm_write_ack(ACK_OK, res);
					break;
				}
				sd_access_factory_reset(false);
				msc_comm_write_ack(ACK_OK, res);
				break;
			case HANDLE_SET_CONFIG:
				if (data_locked || data_mounted) {
					msc_comm_write_ack(ACK_BAD, res);
					break;
				}
				if (msc_comm_read_config()) {
					encrypt_config_t *config_ptr = NULL;
					security_get_config(&config_ptr);
					security_password_reset(config_ptr->encryption_level, NULL);
					msc_comm_write_ack(ACK_OK, res);
				} else {
					msc_comm_write_ack(ACK_BAD, res);
				}
				break;
			case HANDLE_GET_CONFIG:
				msc_comm_write_config(res);
				msc_comm_write_ack(ACK_OK, res);
				break;
			case HANDLE_INPUT_PASS:
				if (!data_locked) {
					security_memset(password_buf, 0, MAX_PASS_LENGTH);
					msc_comm_write_ack(ACK_OK, res);
					break;
				}
				encrypt_config_t *config_ptr = NULL;
				security_get_config(&config_ptr);
				if (config_ptr->encryption_level == 0) {
					sd_access_unlock_data();
					msc_comm_write_ack(ACK_OK, res);
					break;
				} else if (config_ptr->encryption_level == 1) {
					read(cmd, password_buf, MAX_PASS_LENGTH);
					if (security_validate_pass(password_buf, MAX_PASS_LENGTH)) {
						msc_comm_write_ack(ACK_OK, res);
					} else {
						msc_comm_write_ack(ACK_BAD, res);
						break;
					}
				} else {
					msc_comm_write_ack(ACK_BAD, res);
					break;
				}
				security_memset(password_buf, 0, MAX_PASS_LENGTH);
				break;
			case HANDLE_UNLOCK:
				if (!data_locked) {
					sd_access_mount_data();
					msc_comm_write_ack(ACK_OK, res);
					break;
				} else {
					msc_comm_write_ack(ACK_BAD, res);
				}
				break;
			case HANDLE_SET_PASS: {
				if (data_locked || data_mounted) {
					msc_comm_write_ack(ACK_BAD, res);
					break;
				}
				encrypt_config_t *config_ptr = NULL;
				security_get_config(&config_ptr);
				if (config_ptr->encryption_level < MAX_FACTOR) {
					read(cmd, password_buf, MAX_PASS_LENGTH);
					security_write_pass(password_buf, MAX_PASS_LENGTH);
					security_hash_aes_key(password_buf, MAX_PASS_LENGTH);
					security_memset(password_buf, 0, MAX_PASS_LENGTH);
				} else {
					msc_comm_write_ack(ACK_BAD, res);
					break;
				}
				msc_comm_write_ack(ACK_OK, res);
				break;
			}
			case HANDLE_ENCRYPT_QUERY:
				msc_comm_write_ack('?', res);
				if (data_locked) {
					msc_comm_write_ack(ACK_LOCKED, res);
					} else {
					msc_comm_write_ack(ACK_UNLOCKED, res);
				}
				break;
			case HANDLE_UNMOUNT:
				if (!data_mounted) {
					msc_comm_write_ack(ACK_OK, res);
					break;
				}
				sd_access_unmount_data();
				msc_comm_write_ack(ACK_OK, res);
				break;
			case HANDLE_RELOCK:
				if (data_locked) {
					msc_comm_write_ack(ACK_OK, res);
					break;
				}
				sd_access_unmount_data();
				sd_access_lock_data();
				msc_comm_write_ack(ACK_OK, res);
				security_memset(password_buf, 0, MAX_PASS_LENGTH);
				break;
			default:
				msc_comm_write_ack(ACK_BAD, res);
		}
	}
	close(cmd);
	unlink(CMDFILE);
	close(res);
}