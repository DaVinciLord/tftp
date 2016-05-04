#ifndef SERVER_H_
#define SERVER_H_

#include "AdresseInternetType.h"

#define TFTP_PORT 6969


typedef void *(*start_routine_type)(void *);
void *run(void *arg);

typedef struct {
	AdresseInternet *client_addr;
	char *filename;
}thread_args_type;

#endif // SERVER_H_
