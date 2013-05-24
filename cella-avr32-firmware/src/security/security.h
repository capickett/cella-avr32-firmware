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

#define user_data (&user_data_st)

/* Function declarations */

void flash_init(void);

bool validate_pass(uint8_t *password);

void write_pass(uint8_t *password);

#endif /* SECURITY_H_ */