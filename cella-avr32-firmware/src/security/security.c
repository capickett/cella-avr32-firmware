/*
 * security.c
 *
 * Created: 5/21/2013 11:47:09 AM
 *  Author: administrator
 */ 

#include <asf.h>
#include <string.h>
#include "security.h"
#include "flashc.h"

static uint8_t hash_buf[HASH_LENGTH];

// TODO: Generate random salt
static const uint32_t default_salt[SALT_LENGTH/4] = {
	0x31323334,
	0x35363738,
	0x31323334,
	0x35363738,
	0x31323334,
	0x35363738,
	0x31323334,
	0x35363738
};

static void flash_write_user_hash(uint8_t *hash)
{
	flashc_memcpy((uint8_t *)user_data->hash, hash, HASH_LENGTH, true);
}

static void flash_write_user_salt(uint8_t *salt)
{
	flashc_memcpy((uint8_t *)user_data->salt, salt, SALT_LENGTH, true);
}

// TODO: Remove len argument
static void hash_pass(uint8_t *password, uint8_t *salt, uint8_t *output)
{
	uint8_t buf[SALT_LENGTH + MAX_PASS_LENGTH];
	memcpy(buf, salt, SALT_LENGTH);
	memcpy(buf + SALT_LENGTH, password, MAX_PASS_LENGTH);
	sha2(buf, SALT_LENGTH + MAX_PASS_LENGTH, output, 0);
}

void flash_init()
{
	//if (!flashc_is_security_bit_active())
		//flashc_activate_security_bit();
}

// TODO: Remove len argument
bool validate_pass(uint8_t *password)
{
	hash_pass(password, (uint8_t*) user_data->salt, hash_buf);
	return !strncmp((const char*) hash_buf, (const char*)user_data->hash, HASH_LENGTH);
}

//TODO: Remove len argument
void write_pass(uint8_t *password)
{
	hash_pass(password, (uint8_t*) default_salt, hash_buf);
	flash_write_user_hash(hash_buf);
	flash_write_user_salt((uint8_t*) default_salt);
}