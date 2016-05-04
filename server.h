#ifndef SERVER_H_
#define SERVER_H_

#include "socketUDP.h"
#include "AdresseInternetType.h"
#include "options.h"

#define TFTP_PORT 6969

typedef struct {
	AdresseInternet *client_addr;
	char filename[256];
}thread_args_1;

typedef struct {
	AdresseInternet *client_addr;
	char filename[256];
	options *opts;
}thread_args_2;

typedef void *(*start_routine_type)(void *);
void *run1(thread_args_1 *arg);
void *run2(thread_args_2 *arg);

#endif // SERVER_H_
