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

SocketUDP *createSocketUDP();
int initSocketUDP(SocketUDP *psocket);
int attacherSocketUDP(SocketUDP *sock, const char *address, uint16_t port, int flags);
int estAttachee(SocketUDP *socket);
int getLocalName(SocketUDP *socket, char *buffer, int taille);
int getLocalIP(const SocketUDP *socket, char *localIP, int tailleIP);
uint16_t getLocalPort (const SocketUDP *socket);
int writeToSocketUDP(SocketUDP *socket, const AdresseInternet *dst, const char *buffer, size_t length);
int recvFromSocketUDP(SocketUDP *socket, char *response, size_t replength, AdresseInternet *connexion, int timeout);
int closeSocketUDP(SocketUDP *socket);

#endif // SOCKETUDP_H_
