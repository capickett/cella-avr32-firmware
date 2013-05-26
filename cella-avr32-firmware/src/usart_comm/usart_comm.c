/*
 * usart_comm.c
 *
 * Created: 5/21/2013 4:26:47 PM
 *  Author: administrator
 */ 

#include <asf.h>
#include "usart_comm.h"
#include "security.h"
#include "sd_access.h"
#include "flashc.h"
#include "aes.h"

#define HANDLE_SET_CONFIG		'c'
#define HANDLE_GET_CONFIG		'g'
#define HANDLE_INPUT_PASS		'p'
#define HANDLE_SET_PASS			'n'
#define HANDLE_ENCRYPT_QUERY	'?'
#define HANDLE_TOGGLE_MOUNT		't'
#define ACK_OK					'K'
#define ACK_BAD					'~'
#define ACK_UNLOCKED			'U'
#define ACK_LOCKED				'L'
	
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

static void usart_comm_read_password(uint8_t *passwd) {
	int i;
	for (i = 0; i < MAX_PASS_LENGTH; ++i) {
		passwd[i] = usart_getchar(USART_BT);
	}
}

static bool usart_comm_read_config(encrypt_config_t *config) {
	//uint8_t config_string[sizeof(*config)];
	//encrypt_config_t *config_ptr;
	//int i;
	//int max = sizeof(*config);
	//for (i = 0; i < max; ++i) {
		//config_string[i] = usart_getchar(USART_BT);
	//}
	//
	//// Read config from byte string
	//config->encryption_level = config_string[0];
	//
	//if (config->encryption_level != !!config->encryption_level)
		//return false;
	//
	//security_get_user_config(config_ptr);
	//if (config->encryption_level != config_ptr->encryption_level)
		//sd_change_encryption(SD_SLOT_INDEX, config->encryption_level, false, NULL, NULL);
		//
	//flashc_memcpy(config_ptr, config, sizeof(*config), true);
	encrypt_config_t *config_ptr = NULL;
	encrypt_config_t new_config;
	security_get_user_config(&config_ptr);
		
	uint8_t encryption_char = usart_getchar(USART_BT);
	uint8_t encrypt_level = (encryption_char == '1') ? 1 : 0;
	
	if (config_ptr->encryption_level != encrypt_level) {
		//if (!sd_change_encryption(SD_SLOT_INDEX, encrypt_level, false, NULL, NULL))
			//return false;
		new_config.encryption_level = encrypt_level;
		flashc_memcpy(config_ptr, &new_config, sizeof(new_config), true);	
	}			
	return true;
}

static bool usart_comm_write_config(void) {
	encrypt_config_t *config_ptr = NULL;
	security_get_user_config(&config_ptr);
	uint8_t *config_byte_ptr = (uint8_t *)config_ptr;
	int i;
	for (i = 0; i < sizeof(*config_ptr); ++i) {
		usart_write_char(USART_BT, config_byte_ptr[i]);
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
	int c;
	uint8_t passwd[MAX_PASS_LENGTH];
	encrypt_config_t config;
	c = usart_getchar(USART_BT);

	switch (c) {
		case HANDLE_SET_CONFIG:
			usart_comm_read_password(passwd);
			if (security_validate_pass(passwd)) {
				usart_write_char(USART_BT, ACK_OK);
				if (data_mounted)
					unmount_data();
				if (usart_comm_read_config(&config)) {
					usart_write_char(USART_BT, ACK_OK);
				} else {
					usart_write_char(USART_BT, ACK_BAD);
				}
			} else {
				usart_write_char(USART_BT, ACK_BAD);
			}
			break;
		case HANDLE_GET_CONFIG:
			usart_comm_read_password(passwd);
			if (security_validate_pass(passwd)) {
				usart_write_char(USART_BT, ACK_OK);
				if (usart_comm_write_config()) {
					usart_write_char(USART_BT, ACK_OK);
				} else {
					usart_write_char(USART_BT, ACK_BAD);
				}
			} else {
				usart_write_char(USART_BT, ACK_BAD);
			}
			break;
		case HANDLE_INPUT_PASS:	{
			if (data_mounted) {
				usart_write_char(USART_BT, ACK_BAD);
				break;
			}
			usart_comm_read_password(passwd);
			if (unlock_drive(passwd)) {
				usart_write_char(USART_BT, ACK_OK);
			} else {
				usart_write_char(USART_BT, ACK_BAD);
			}
			break;
		}
		case HANDLE_SET_PASS: {
			uint8_t old_passwd[MAX_PASS_LENGTH];
			uint8_t new_passwd[MAX_PASS_LENGTH];
			usart_comm_read_password(old_passwd);
			if (security_validate_pass(old_passwd)) {
				usart_write_char(USART_BT, ACK_OK);
				if (data_mounted)
					unmount_data();
				usart_comm_read_password(new_passwd);
				//if (sd_change_encryption(SD_SLOT_INDEX, false, true, old_passwd, new_passwd) == CTRL_GOOD) {
				security_write_pass(new_passwd);
				usart_write_char(USART_BT, ACK_OK);
				//} else {
					//usart_write_char(USART_BT, ACK_BAD);
				//}
			} else {
				usart_write_char(USART_BT, ACK_BAD);
			}
			break;
		}
		case HANDLE_ENCRYPT_QUERY:
			usart_putchar(USART_BT, '?');
			if (data_mounted) {
				usart_putchar(USART_BT, ACK_UNLOCKED);
			} else {
				usart_putchar(USART_BT, ACK_LOCKED);
			}
			break;
		case HANDLE_TOGGLE_MOUNT:
			if (data_mounted) {
				unmount_data();
			} else {
				mount_data();
			}
			break;
		default:
			usart_write_char(USART_BT, ACK_BAD);
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
