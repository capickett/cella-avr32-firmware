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

static int fd;

void msc_comm_init(void) {
	b_fsaccess_init();
	const uint8_t _MEM_TYPE_SLOW_ script_name[17] = 
		{'.', 'c', 'e', 'l', 'l', 'a', '_', 'u', 'n', 'l', 'o', 'c', 'k', '.', 's', 'h', 0};
	
	if (nav_drive_set(LUN_ID_VIRTUAL_MEM)) {
		nav_drive_format(FS_FORMAT_DEFAULT);
	
		// TODO: get script into static array
		uint8_t _MEM_TYPE_SLOW_* script_buf;
		uint16_t script_size;
		if (nav_file_create( (const FS_STRING) script_name ) && file_open(FOPEN_MODE_W)) {
			file_write_buf(script_buf, script_size);
		}
		file_close();
	}
}

bool file_exists(void) {
	int lockfd;
	if ((lockfd = open(LOCKFILE, O_RDONLY) >= 0)) {
		close(lockfd);
		return false;
	}
	return ((fd = open(CMDFILE, O_RDONLY) >= 0));
}

void process_file(void) {
	int cmd = fd;
	int res = open(RESFILE, O_WRONLY);
	if ((cmd >= 0) && (res >= 0)) {
		uint8_t cmd_buf;
		read(cmd, &cmd_buf, 1);
		switch (cmd_buf) {
		case 'p': {
			uint8_t pass_buf[MAX_PASS_LENGTH];
			read(cmd, pass_buf, MAX_PASS_LENGTH);

			uint8_t resp;
			if (security_validate_pass(pass_buf))
				resp = 'K';
			else
				resp = '~';
			write(res, &resp, 1);
			break;
		}
		case 'g': {
			encrypt_config_t *config;
			security_get_user_config(&config);
			write(res, &(config->encryption_level), 1);
			break;
		}
		case 'c': {
			encrypt_config_t config;
			uint8_t encryption_level;
			read(cmd, &encryption_level, 1);
			config.encryption_level = encryption_level;
			security_write_user_config(&config);
			break;
		}
		case 'n': {
			uint8_t pass_buf[MAX_PASS_LENGTH];
			read(cmd, pass_buf, MAX_PASS_LENGTH);
			char resp;
			if (security_validate_pass(pass_buf)) {
				read(cmd, pass_buf, MAX_PASS_LENGTH);
				security_write_pass(pass_buf);
				resp = 'K';
			} else {
				resp = '~';
			}
			write(res, &resp, 1);
			break;
		}
		}
	}
	close(cmd);
	unlink(CMDFILE);
	close(res);
}