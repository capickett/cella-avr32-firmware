/*
 * usart_comm.c
 *
 * Created: 5/21/2013 4:26:47 PM
 *  Author: administrator
 */ 

#include <asf.h>
#include "usart_comm.h"
#include "conf_usart_comm.h"
#include "security.h"
#include "data_mount.h"

static const gpio_map_t USART_GPIO_MAP = {
	{ USART_RX_PIN, USART_RX_FUNCTION },
	{ USART_TX_PIN, USART_TX_FUNCTION }
};

static usart_serial_options_t usart_options = {
	.baudrate = USART_SERIAL_BAUDRATE,
	.charlength = USART_SERIAL_CHAR_LENGTH,
	.paritytype = USART_SERIAL_PARITY,
	.stopbits = USART_SERIAL_STOP_BIT,
	.channelmode = CONFIG_USART_SERIAL_MODE
};

//static int read_password(void) {
	//char passwd[MAX_PASS_LENGTH];
	//int i;
	//for (i = 0; i < MAX_PASS_LENGTH; ++i)
	//passwd[i] = getchar();
	//return unlock_drive(passwd);
//}

//static int unlock_drive(char* passwd) {
	//if (validate_pass(passwd)) {
		//return mount_data();
	//}
//}

//static void read_config(encrypt_config *config) {
	//char config_string[sizeof(*config)];
	//int i;
	//int max = sizeof(*config);
	//for (i = 0; i < max; ++i) {
		//config_string[i] = getchar();
	//}
	//config->encryption_level = (uint8_t) config_string[0];
	///* other config options get parsed here */
//}

/* USART Setup */
void usart_comm_init()
{
	gpio_enable_module(USART_GPIO_MAP, sizeof(USART_GPIO_MAP)/ sizeof(USART_GPIO_MAP[0]));
	usart_serial_init(USART_SERIAL, &usart_options);
}

/* process data */
//int process_data() {
	//char c = getchar();
//
	//switch (c) {
		//case 'c':
			//config_st config;
			//read_config(stream, &config);
			//// send back 'K' as okay response
			//return set_config(config);
		//case 'C':
			//// send my config
			//break;
		//case 'p':
			//// send back 'K' as okay response
			//return read_password(stream);
		//default:
			//fprintf(stderr, "Bad character");
			//return -1;
	//}
//}
//