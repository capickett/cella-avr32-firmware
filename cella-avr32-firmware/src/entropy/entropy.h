/*
 * entropy.h
 *
 * Created: 5/28/2013 4:44:21 PM
 *  Author: administrator
 */ 


#ifndef ENTROPY_H_
#define ENTROPY_H_

void entropy_init(void);

bool get_entropy(uint8_t *entropy_buf, uint8_t bytes);

#endif /* ENTROPY_H_ */