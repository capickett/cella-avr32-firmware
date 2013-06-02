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
#include "entropy.h"

//#if defined (__GNUC__)
//__attribute__((__section__(".userpage")))
//#elif defined(__ICCAVR32__)
//__no_init
//#endif
//static user_data_t user_data_st
//#if defined (__ICCAVR32__)
//@ "USERDATA32_C"
//#endif
//;

#define USER_PAGE_ST	((user_data_t *)AVR32_FLASHC_USER_PAGE_ADDRESS)

static uint8_t hash_buf[HASH_LENGTH];
static uint8_t salt_buf[SALT_LENGTH];
static uint8_t hash_salt_buf[MAX_PASS_LENGTH + SALT_LENGTH];
static uint8_t hash_buf_cipher[HASH_LENGTH];

void security_flash_write_hash(uint8_t *hash)
{
	flashc_memcpy(USER_PAGE_ST->hash, hash, HASH_LENGTH, true);
}

void security_flash_write_salt(uint8_t *salt)
{
	flashc_memcpy(USER_PAGE_ST->salt, salt, SALT_LENGTH, true);
}

void security_flash_write_config(encrypt_config_t *config_ptr)
{
	flashc_memcpy(&(USER_PAGE_ST->config), config_ptr, sizeof(*config_ptr), true);
}

void security_flash_write_factory_reset(bool reset)
{
	flashc_memcpy(&(USER_PAGE_ST->factory_reset), &reset, sizeof(bool), true);
}

void hash_pass_salt(uint8_t *password, uint8_t *salt, uint8_t *output)
{
	memcpy(hash_salt_buf, salt, SALT_LENGTH);
	memcpy(hash_salt_buf + SALT_LENGTH, password, MAX_PASS_LENGTH);
	sha2(hash_salt_buf, SALT_LENGTH + MAX_PASS_LENGTH, output, 0);
	security_memset(hash_salt_buf, 0, SALT_LENGTH + MAX_PASS_LENGTH);
}

void security_flash_init()
{
	//if (!flashc_is_security_bit_active())
		//flashc_activate_security_bit();
}

void security_factory_reset_init()
{
	if (security_get_factory_reset()) {
		sd_access_factory_reset(true);
	}
}

void security_password_reset()
{
	uint8_t blank_pass_buf[MAX_PASS_LENGTH];
	security_memset(blank_pass_buf, 0, MAX_PASS_LENGTH);
	security_write_pass(blank_pass_buf);
}

void security_user_config_reset()
{
	encrypt_config_t default_st;
	default_st.encryption_level = 1;
	security_flash_write_config(&default_st);
}

bool security_validate_pass(uint8_t *password)
{
	hash_pass_salt(password, (uint8_t*) USER_PAGE_ST->salt, hash_buf);
	if (!strncmp((const char*) hash_buf, (const char*)USER_PAGE_ST->hash, HASH_LENGTH)) {
		security_memset(hash_buf, 0, HASH_LENGTH);
		hash_aes_key(password);
		sd_access_unlock_data();
		return true;
	}
	security_memset(hash_buf, 0, HASH_LENGTH);
	return false;
}

void hash_aes_key(uint8_t *password)
{
	sha2(password, MAX_PASS_LENGTH, hash_buf_cipher, 0);
	aes_set_key(&AVR32_AES, (unsigned int *)hash_buf_cipher);
	security_memset(hash_buf_cipher, 0, HASH_LENGTH);
}

void security_write_pass(uint8_t *password)
{
	get_entropy(salt_buf, SALT_LENGTH);
	hash_pass_salt(password, (uint8_t*) salt_buf, hash_buf);
	security_flash_write_hash(hash_buf);
	security_flash_write_salt((uint8_t*) salt_buf);
	security_memset(hash_buf, 0, HASH_LENGTH);
	security_memset(salt_buf, 0, SALT_LENGTH);
}

void security_get_hash(uint8_t **hash_ptr)
{
	*hash_ptr = USER_PAGE_ST->hash;
}

void security_get_salt(uint8_t **salt_ptr)
{
	*salt_ptr = USER_PAGE_ST->salt;
}

void security_get_config(encrypt_config_t **config_ptr)
{
	*config_ptr = &(USER_PAGE_ST->config);
}

bool security_get_factory_reset()
{
	return USER_PAGE_ST->factory_reset;
}

void *security_memset(void *v, int c, size_t n)
{
	volatile unsigned char *p = v;
	while (n--) *p++ = c;
	return v;
}