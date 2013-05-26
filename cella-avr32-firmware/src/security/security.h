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

typedef struct user_data_struct
{
	encrypt_config_t config;
	uint8_t hash[HASH_LENGTH];
	uint8_t salt[SALT_LENGTH];
} user_data_t;

/* Function declarations */

void security_flash_init(void);

bool security_validate_pass(uint8_t *password);

void security_write_pass(uint8_t *password);

void security_write_config(encrypt_config_t *config_ptr);

void security_get_user_hash(uint8_t **hash_ptr);

void security_get_user_salt(uint8_t **salt_ptr);

void security_get_user_config(encrypt_config_t **config_ptr);

#endif /* SECURITY_H_ */