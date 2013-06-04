/*
 * usart_comm.h
 *
 * Created: 5/21/2013 4:27:41 PM
 *  Author: administrator
 */ 

#ifndef USART_COMM_H_
#define USART_COMM_H_

typedef struct __attribute__((packed)) encrypt_config_st {
	uint8_t encryption_level;
} encrypt_config_t;

void usart_comm_init(void);

void usart_comm_bt_reset(void);

#endif /* USART_COMM_H_ */