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

static bool unlock_drive(uint8_t* passwd) {
	if (validate_pass(passwd)) {
		// TODO: hash password and set as AES cipher key
		mount_data();
		return true;
	} else {
		return false;
	}
}

static bool read_password(void) {
	uint8_t passwd[MAX_PASS_LENGTH];
	int i;
	for (i = 0; i < MAX_PASS_LENGTH; ++i) {
		passwd[i] = usart_getchar(USART_BT);
	}
	return unlock_drive((uint8_t*) passwd);
}

static bool read_config(encrypt_config_t *config) {
	uint8_t config_string[sizeof(*config)];
	int i;
	int max = sizeof(*config);
	for (i = 0; i < max; ++i) {
		config_string[i] = usart_getchar(USART_BT);
	}
	
	// Read config from byte string
	config->encryption_level = config_string[0];
	
	// TODO: validate and write config to userpage
	if (config->encryption_level != !!config->encryption_level)
		return false;
		
	flashc_memcpy(&user_data->config, config, sizeof(*config), true);
	
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
	encrypt_config_t config;
	c = usart_getchar(USART_BT);

	switch (c) {
	case 'c':
		if (read_config(&config)) {
			usart_write_char(USART_BT, 'K');
		} else {
			usart_write_char(USART_BT, '~');
		}
		break;
	case 'C':
		usart_write_char(USART_BT, 'K');
		break;
	case 'p':
		if (read_password()) {
			usart_write_char(USART_BT, 'K');
		} else {
			usart_write_char(USART_BT, '~');
		}
		break;
	default:
		usart_write_char(USART_BT, '~');
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
