#ifndef SOCKETUDP_H_
#define SOCKETUDP_H_

#include <stdint.h>
#include "AdresseInternetType.h"

typedef struct {
	int sockfd;
	AdresseInternet *addr;
	int is_bound;
} _socketUDP_struct;

typedef _socketUDP_struct SocketUDP;

int initSocketUDP(SocketUDP *psocket);
int attacherSocketUDP(SocketUDP *sock, const char *address, uint16_t port, int flags);
int estAttachee(SocketUDP *socket);
#endif // SOCKETUDP_H_
