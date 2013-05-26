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
#include "sd_access.h"

#if defined (__GNUC__)
__attribute__((__section__(".userpage")))
#elif defined(__ICCAVR32__)
__no_init
#endif
static user_data_t user_data_st
#if defined (__ICCAVR32__)
@ "USERDATA32_C"
#endif
;

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
	flashc_memcpy((uint8_t *)user_data_st.hash, hash, HASH_LENGTH, true);
}

static void flash_write_user_salt(uint8_t *salt)
{
	flashc_memcpy((uint8_t *)user_data_st.salt, salt, SALT_LENGTH, true);
}

static void hash_pass_salt(uint8_t *password, uint8_t *salt, uint8_t *output)
{
	uint8_t buf[SALT_LENGTH + MAX_PASS_LENGTH];
	memcpy(buf, salt, SALT_LENGTH);
	memcpy(buf + SALT_LENGTH, password, MAX_PASS_LENGTH);
	sha2(buf, SALT_LENGTH + MAX_PASS_LENGTH, output, 0);
}

void security_flash_init()
{
	//if (!flashc_is_security_bit_active())
		//flashc_activate_security_bit();
}

bool security_validate_pass(uint8_t *password)
{
	hash_pass_salt(password, (uint8_t*) user_data_st.salt, hash_buf);
	return !strncmp((const char*) hash_buf, (const char*)user_data_st.hash, HASH_LENGTH);
}

void security_write_pass(uint8_t *password)
{
	hash_pass_salt(password, (uint8_t*) default_salt, hash_buf);
	flash_write_user_hash(hash_buf);
	flash_write_user_salt((uint8_t*) default_salt);
}

void security_write_config(encrypt_config_t *config_ptr)
{
	flashc_memcpy((encrypt_config_t *)(&user_data_st.config), config_ptr, sizeof(*config_ptr), true);
}

void security_get_user_hash(uint8_t **hash_ptr)
{
	*hash_ptr = user_data_st.hash;
}

void security_get_user_salt(uint8_t **salt_ptr)
{
	*salt_ptr = user_data_st.salt;
}

void security_get_user_config(encrypt_config_t **config_ptr)
{
	*config_ptr = &user_data_st.config;
}