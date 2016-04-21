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
int writeToSocketUDP(SocketUDP *socket, const AdresseInternet *dst, char *buffer, size_t length);
int recvFromSocketUDP(SocketUDP *socket, char *response, size_t replength, AdresseInternet *connexion, int timeout);
#endif // SOCKETUDP_H_
