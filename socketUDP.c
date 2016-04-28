#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>

#include "socketUDP.h"
#include "AdresseInternetType.h"
int initSocketUDP(SocketUDP *psocket) {
	psocket->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(psocket->sockfd == -1) {
		return -1;
	}
	
	return 0;
}




int attacherSocketUDP(SocketUDP *sock, const char *address, uint16_t port, int flags) {

	if (address == NULL) {
		switch (flags) {
			case INADDR_LOOPBACK : 
				sock->addr = AdresseInternet_new("127.0.0.1", port);
				break;
			case 0 : 
				sock->addr = AdresseInternet_new(INADDR_ANY, port);
				break;
			default : 
				return -1;
		
		}
	} else {
		sock->addr = AdresseInternet_new(address, port);
	}
	if(bind(sock->sockfd, (struct sockaddr *) &sock->addr->sockAddr, sizeof(sock->addr->sockAddr)) == -1) {
		return -1;
	} 
	sock->is_bound = 1;
	
	return 0;
}

int estAttachee(SocketUDP *socket) {
	return (socket->is_bound == 1) ? 0 : -1 ;
}

int writeToSocketUDP(SocketUDP *socket, const AdresseInternet *dst, char *buffer, size_t length) {
	if(socket == NULL || dst == NULL || buffer == NULL) {
        return -1;
    }
    struct sockaddr sockAddr;
    AdresseInternet_to_sockaddr(dst, &sockAddr);
    return sendto(socket->sockfd, buffer, (size_t)length, 0, &sockAddr, sizeof(sockAddr));

	
	
}

int recvFromSocketUDP(SocketUDP *socket, char *response, size_t replength, AdresseInternet *connexion, int timeout) {
if (socket == NULL &&
response == NULL &&
replength == 0 &&
connexion == NULL &&
timeout == 0) printf("Werror sucks"); 
return 1;
}


