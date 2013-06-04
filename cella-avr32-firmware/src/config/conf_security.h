/*
 * conf_security.h
 *
 * Created: 6/2/2013 11:19:53 PM
 *  Author: administrator
 */ 


#ifndef CONF_SECURITY_H_
#define CONF_SECURITY_H_

#define MIN_FACTOR		0
#define MAX_FACTOR		2

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

#endif /* CONF_SECURITY_H_ */