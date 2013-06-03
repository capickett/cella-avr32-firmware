/*
 * msc_comm.h
 *
 * Created: 5/30/2013 2:45:23 PM
 *  Author: administrator
 */ 


#ifndef MSC_COMM_H_
#define MSC_COMM_H_

#include <stdbool.h>

#define RESFILE  "__cellaRES"
#define CMDFILE  "__cellaCMD"
#define LOCKFILE "__cellaLOCK"

void msc_comm_init(void);

bool file_exists(void);

void process_file(void);

#endif /* MSC_COMM_H_ */