#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>


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
    if(socket == NULL || connexion == NULL || response == NULL) {
        return (ssize_t)-1;
    }
    struct sockaddr sockAddr;
    memset(&sockAddr, 0, sizeof(struct sockaddr));
    socklen_t *addr_length = (socklen_t*)malloc(sizeof(*addr_length));
    *addr_length = sizeof(struct sockaddr);
    int nfds;
    fd_set read_fds;
    struct timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    FD_ZERO(&read_fds);
    FD_SET(socket->sockfd, &read_fds);
    nfds = (socket->sockfd+1);
    int result = select(nfds, &read_fds, NULL, NULL, &tv);
    if(result == -1) {
        perror("Select : ");
        return -1;
    }
    ssize_t size = 0;
    if(FD_ISSET(socket->sockfd, &read_fds)) {
       size = recvfrom(socket->sockfd, response, (size_t)replength, 0, &sockAddr, addr_length);
    }
    sockaddr_to_AdresseInternet(&sockAddr, connexion); 
    free(addr_length);
    return size;
}


