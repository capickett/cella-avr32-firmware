/*
 * usart_comm.h
 *
 * Created: 5/21/2013 4:27:41 PM
 *  Author: administrator
 */ 

#ifndef USART_COMM_H_
#define USART_COMM_H_

typedef struct config_st {
	uint8_t encryption_level;
} encrypt_config;

void usart_comm_init(void);

#endif /* USART_COMM_H_ */