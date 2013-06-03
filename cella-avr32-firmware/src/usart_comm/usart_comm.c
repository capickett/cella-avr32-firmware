/*
 * usart_comm.c
 *
 * Created: 5/21/2013 4:26:47 PM
 *  Author: administrator
 */ 

#include <asf.h>
#include <string.h>
#include "conf_security.h"
#include "conf_factory.h"
#include "usart_comm.h"
#include "security.h"
#include "sd_access.h"
#include "flashc.h"
#include "aes.h"
#include "delay.h"

#define HANDLE_SET_CONFIG		'c'
#define HANDLE_GET_CONFIG		'g'
#define HANDLE_INPUT_PASS		'p'
#define HANDLE_UNLOCK			'k'
#define HANDLE_SET_PASS			'n'
#define HANDLE_ENCRYPT_QUERY	'?'
#define HANDLE_RELOCK			'l'
#define HANDLE_RESET			'r'
#define HANDLE_UNMOUNT			'u'
#define ACK_OK					'K'
#define ACK_BAD					'~'
#define ACK_UNLOCKED			'U'
#define ACK_LOCKED				'L'
	
static uint8_t password_buf[MAX_PASS_LENGTH];
static uint8_t uuid_buf[UUID_LENGTH];
static uint8_t multi_buf[MAX_PASS_LENGTH + UUID_LENGTH];

static const gpio_map_t USART_BT_GPIO_MAP = {
	{ USART_BT_RX_PIN, USART_BT_RX_FUNCTION },
	{ USART_BT_TX_PIN, USART_BT_TX_FUNCTION }
};

static usart_serial_options_t usart_options = {
	.baudrate = USART_BT_BAUDRATE,
	.charlength = USART_BT_CHAR_LENGTH,
	.paritytype = USART_BT_PARITY,
	.stopbits = USART_BT_STOP_BIT,
	.channelmode = CONFIG_USART_BT_SERIAL_MODE
};

static void usart_comm_read_string(uint8_t length, uint8_t *buf) {
	int i;
	for (i = 0; i < length; ++i) {
		buf[i] = usart_getchar(USART_BT);
	}
}

static bool usart_comm_read_config(void) {
	int i, max;
	uint8_t config_string[sizeof(encrypt_config_t)];
	encrypt_config_t *config_ptr = NULL;
	encrypt_config_t new_config;
	security_get_config(&config_ptr);
	
	//max = sizeof(encrypt_config_t);
	//for (i = 0; i < max; ++i) {
		//config_string[i] = usart_getchar(USART_BT);
	//}
		
	//uint8_t encrypt_level = ((encrypt_config_t *)config_string)->encryption_level;
	char encrypt_char = usart_getchar(USART_BT);
	uint8_t encrypt_level = 0;
	if (encrypt_char == '1') {
		encrypt_level = 1;
	} else if (encrypt_char == '2') {
		encrypt_level = 2;
	}
	new_config.encryption_level = encrypt_level;
	
	if (encrypt_level > MAX_FACTOR || encrypt_level < MIN_FACTOR)
		return false;
	
	if (config_ptr->encryption_level != encrypt_level) {
		//security_flash_write_config((encrypt_config_t *)config_string);
		security_flash_write_config((encrypt_config_t *)&new_config);	
	}
				
	return true;
}

static bool usart_comm_write_config(void) {
	encrypt_config_t *config_ptr = NULL;
	security_get_config(&config_ptr);
	uint8_t *config_byte_ptr = (uint8_t *)config_ptr;
	int i;
	//for (i = 0; i < sizeof(*config_ptr); ++i) {
		//usart_putchar(USART_BT, config_byte_ptr[i]);
	//}
	if (!config_ptr->encryption_level) {
		usart_putchar(USART_BT, '0');
	} else if (config_ptr->encryption_level == 1) {
		usart_putchar(USART_BT, '1');
	} else if (config_ptr->encryption_level == 2) {
		usart_putchar(USART_BT, '2');
	}
	return true;
}

/* process data */
#if __GNUC__
__attribute__((__interrupt__))
#elif __ICCAVR32__
	__interrupt
#endif
static void process_data(void) {
	LED_Toggle(LED0);
	volatile int c;
	c = usart_getchar(USART_BT);

	switch (c) {
		case USART_FAILURE:
			break;
		case HANDLE_RESET:
			if (data_locked) {
				usart_putchar(USART_BT, ACK_BAD);
				break;
			}
			sd_access_factory_reset(false);
			usart_putchar(USART_BT, ACK_OK);
			break;
		case HANDLE_SET_CONFIG:
			if (data_locked) {
				usart_putchar(USART_BT, ACK_BAD);
				break;
			}
			if (usart_comm_read_config()) {
				encrypt_config_t *config_ptr = NULL;
				security_get_config(&config_ptr);
				if (config_ptr->encryption_level == 2)
					usart_comm_read_string(UUID_LENGTH, uuid_buf);
				security_password_reset(config_ptr->encryption_level, uuid_buf);
				security_memset(uuid_buf, 0, UUID_LENGTH);
				usart_putchar(USART_BT, ACK_OK);
			} else {
				usart_putchar(USART_BT, ACK_BAD);
			}
			break;
		case HANDLE_GET_CONFIG:
			if (data_locked) {
				usart_putchar(USART_BT, ACK_BAD);
				break;
			}
			usart_comm_write_config();
			usart_putchar(USART_BT, ACK_OK);
			break;
		case HANDLE_INPUT_PASS:
			if (!data_locked) {
				security_memset(password_buf, 0, MAX_PASS_LENGTH);
				usart_putchar(USART_BT, ACK_OK);
				break;
			}
			encrypt_config_t *config_ptr = NULL;
			security_get_config(&config_ptr);
			if (config_ptr->encryption_level == 0) {
				sd_access_unlock_data();
				break;
			} else if (config_ptr->encryption_level == 1) {
				usart_comm_read_string(MAX_PASS_LENGTH, password_buf);
				if (security_validate_pass(password_buf, MAX_PASS_LENGTH)) {
					usart_putchar(USART_BT, ACK_OK);
				} else {
					usart_putchar(USART_BT, ACK_BAD);
				}
			} else if (config_ptr->encryption_level == 2) {
				usart_comm_read_string(MAX_PASS_LENGTH, password_buf);
				usart_comm_read_string(UUID_LENGTH, uuid_buf);
				memcpy(multi_buf, uuid_buf, UUID_LENGTH);
				memcpy(multi_buf + UUID_LENGTH, password_buf, MAX_PASS_LENGTH);
				if (security_validate_pass(multi_buf, MAX_PASS_LENGTH + UUID_LENGTH)) {
					usart_putchar(USART_BT, ACK_OK);
				} else {
					usart_putchar(USART_BT, ACK_BAD);
				}
			} else {
				usart_putchar(USART_BT, ACK_BAD);
				break;
			}
			security_memset(password_buf, 0, MAX_PASS_LENGTH);
			security_memset(uuid_buf, 0, UUID_LENGTH);
			security_memset(multi_buf, 0, MAX_PASS_LENGTH + UUID_LENGTH);
			break;
		case HANDLE_UNLOCK:
			if (!data_locked) {
				sd_access_mount_data();
				usart_putchar(USART_BT, ACK_OK);
				break;
			} else {
				usart_putchar(USART_BT, ACK_BAD);
			}
			break;
		case HANDLE_SET_PASS: {
			if (data_locked || data_mounted) {
				usart_putchar(USART_BT, ACK_BAD);
				break;
			}
			encrypt_config_t *config_ptr = NULL;
			security_get_config(&config_ptr);
			if (config_ptr->encryption_level < 2) {
				usart_comm_read_string(MAX_PASS_LENGTH, password_buf);
				security_write_pass(password_buf, MAX_PASS_LENGTH);
				security_hash_aes_key(password_buf, MAX_PASS_LENGTH);
				security_memset(password_buf, 0, MAX_PASS_LENGTH);
			} else if (config_ptr->encryption_level == 2) {
				usart_comm_read_string(MAX_PASS_LENGTH, password_buf);
				usart_comm_read_string(UUID_LENGTH, uuid_buf);
				memcpy(multi_buf, uuid_buf, UUID_LENGTH);
				memcpy(multi_buf + UUID_LENGTH, password_buf, MAX_PASS_LENGTH);
				security_write_pass(multi_buf, MAX_PASS_LENGTH + UUID_LENGTH);
				security_hash_aes_key(multi_buf, MAX_PASS_LENGTH + UUID_LENGTH);
				security_memset(password_buf, 0, MAX_PASS_LENGTH);
				security_memset(uuid_buf, 0, UUID_LENGTH);
				security_memset(multi_buf, 0, MAX_PASS_LENGTH + UUID_LENGTH);
			} else {
				usart_putchar(USART_BT, ACK_BAD);
				break;
			}
			usart_putchar(USART_BT, ACK_OK);
			break;
		}
		case HANDLE_ENCRYPT_QUERY:
			usart_putchar(USART_BT, '?');
			if (data_locked) {
				usart_putchar(USART_BT, ACK_LOCKED);
			} else {
				usart_putchar(USART_BT, ACK_UNLOCKED);
			}
			break;
		case HANDLE_UNMOUNT:
			sd_access_unmount_data();
			usart_putchar(USART_BT, ACK_OK);
			break;
		case HANDLE_RELOCK:
			if (data_locked) {
				usart_putchar(USART_BT, ACK_OK);
				break;
			}
			sd_access_unmount_data();
			sd_access_lock_data();
			usart_putchar(USART_BT, ACK_OK);
			security_memset(password_buf, 0, MAX_PASS_LENGTH);
			break;
		default:
			usart_putchar(USART_BT, ACK_BAD);
	}
}

/* USART Setup */
void usart_comm_init() {
	gpio_enable_module(USART_BT_GPIO_MAP,
			sizeof(USART_BT_GPIO_MAP) / sizeof(USART_BT_GPIO_MAP[0]));

	usart_serial_init(USART_BT, &usart_options);
		
	Disable_global_interrupt();
	
	INTC_init_interrupts();
	INTC_register_interrupt(&process_data, USART_BT_IRQ, AVR32_INTC_INT0);
	
	USART_BT->ier = AVR32_USART_IER_RXRDY_MASK;
	
	Enable_global_interrupt();
}
