/*
 * security.h
 *
 * Created: 5/21/2013 10:52:58 AM
 *  Author: administrator
 */ 


#ifndef SECURITY_H_
#define SECURITY_H_

#include "compiler.h"
#include "usart_comm.h"

/* Constants */

#define HASH_LENGTH 32
#define SALT_LENGTH 32
#define MAX_PASS_LENGTH 32
#define FLASH_SIZE 512

/* Structure of user data in FLASH */
typedef struct __attribute__((packed)) user_data_struct
{
	bool factory_reset;
	encrypt_config_t config;
	uint8_t hash[HASH_LENGTH];
	uint8_t salt[SALT_LENGTH];
} user_data_t;

/* Function declarations */

void security_flash_write_hash(uint8_t *hash);

void security_flash_write_salt(uint8_t *salt);

void security_flash_write_config(encrypt_config_t *config_ptr);

void security_flash_write_factory_reset(bool reset);

void hash_pass_salt(uint8_t *password, uint8_t *salt, uint8_t *output);

void security_flash_init(void);

void security_factory_reset_init(void);

bool security_validate_pass(uint8_t *password);

void security_write_pass(uint8_t *password);

void security_write_factory_reset(bool reset);

void security_get_hash(uint8_t **hash_ptr);

void security_get_salt(uint8_t **salt_ptr);

void security_get_config(encrypt_config_t **config_ptr);

bool security_get_factory_reset(void);

void security_password_reset(void);

void security_user_config_reset(void);

void *security_memset(void *v, int c, size_t n);

void hash_aes_key(uint8_t *password);

#endif /* SECURITY_H_ */