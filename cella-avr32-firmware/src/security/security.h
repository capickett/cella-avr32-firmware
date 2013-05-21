/*
 * security.h
 *
 * Created: 5/21/2013 10:52:58 AM
 *  Author: administrator
 */ 


#ifndef SECURITY_H_
#define SECURITY_H_

#include "compiler.h"

/* Constants */

#define HASH_LENGTH 32
#define SALT_LENGTH 32
#define MAX_PASS_LENGTH 32
#define FLASH_SIZE 512

/* Structure of user data in FLASH */

typedef struct user_data_struct
{
	uint8_t hash[HASH_LENGTH];
	uint8_t salt[SALT_LENGTH];
} USER_DATA_TYPE;

#define USER_PAGE_ADDRESS 0x80800000
#define user_data ((USER_DATA_TYPE *)USER_PAGE_ADDRESS)

/* Function declarations */

void flash_init(void);

bool validate_pass(uint8_t *password, uint8_t len);

void write_pass(uint8_t *password, uint8_t len);

#endif /* SECURITY_H_ */