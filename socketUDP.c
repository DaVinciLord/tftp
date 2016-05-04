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

/**
 * 
 * Projet TFTP
 * Metton Vincent
 * Pommier Grégoire
 * 
 * 
 * */
//On créé une socket
SocketUDP *createSocketUDP() {
    SocketUDP *socket = (SocketUDP *)malloc(sizeof(SocketUDP));
    memset(socket, 0, sizeof(*socket));
    return socket;
}

//On initialise psocket
int initSocketUDP(SocketUDP *psocket) {
	psocket->sockfd = socket(PF_INET, SOCK_DGRAM, 0);
	if(psocket->sockfd == -1) {
		return -1;
	}
	
	return 0;
}

//On bind
int attacherSocketUDP(SocketUDP *sock, const char *address, uint16_t port, int flags) {

	if (address == NULL) {
		switch (flags) {
			case INADDR_LOOPBACK : 
				sock->addr = AdresseInternet_loopback(port);
				break;
			case 0 : 
				sock->addr = AdresseInternet_any(port);
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

//Renvois si la socket est attachée
int estAttachee(SocketUDP *socket) {
	return (socket->is_bound == 1) ? 0 : -1 ;
}

//Renvois le nom local de la socket
int getLocalName(SocketUDP *socket, char *buffer, int taille) {
    if(socket == NULL || socket->addr == NULL || buffer == NULL) {
        return -1;
    }
    AdresseInternet_getinfo(socket->addr, buffer, taille, NULL, 0);
    int n = strlen(buffer);
    return n;
}

//Renvois l'ip locale
int getLocalIP(const SocketUDP *socket, char *localIP, int tailleIP) {
    if(socket == NULL || socket->addr == NULL || localIP == NULL) {
        return -1;
    }
    AdresseInternet_getIP(socket->addr, localIP, tailleIP);
    int n = strlen(localIP);
    return n;
}

//Renvois local Port
uint16_t getLocalPort (const SocketUDP *socket) {
    if(socket == NULL || socket->addr == NULL) {
        return -1;
    }
    return AdresseInternet_getPort(socket->addr);
}

//permet d'ecrire a travers la socket, à l'adreese dst
int writeToSocketUDP(SocketUDP *socket, const AdresseInternet *dst, const char *buffer, size_t length) {
	if(socket == NULL || dst == NULL || buffer == NULL) {
        return -1;
    }
    struct sockaddr sockAddr;
    AdresseInternet_to_sockaddr(dst, &sockAddr);
    return sendto(socket->sockfd, buffer, (size_t)length, 0, &sockAddr, sizeof(sockAddr));
}

//Permet de recevoir depuis la socket
int recvFromSocketUDP(SocketUDP *socket, char *response, size_t replength, AdresseInternet *connexion, int timeout) {
    if(socket == NULL || connexion == NULL || response == NULL) {
        return -1;
    }
    struct sockaddr sockAddr;
    memset(&sockAddr, 0, sizeof(struct sockaddr));
    socklen_t *addr_length = (socklen_t*)malloc(sizeof(*addr_length));
    *addr_length = sizeof(struct sockaddr);
    /**
     * Gestion du timeout
     */
    int nfds;
    fd_set read_fds;
    struct timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    FD_ZERO(&read_fds);
    FD_SET(socket->sockfd, &read_fds);
    nfds = (socket->sockfd + 1);
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

//Ferme la socket
int closeSocketUDP(SocketUDP *socket) {
    if(socket == NULL || socket->addr == NULL) {
        return -1;
    }
    
    int status = close(socket->sockfd);
    AdresseInternet_free(socket->addr);
    free(socket);
    return status;
}


