#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"
#include "socketUDP.h"
#include <unistd.h>

#include "tftp.h"
#define ADDR_SERVER "10.130.162.32"
#define TFTP_PORT 6969
#define FILENAME "lenarraypourlesnuls.txt"

int main(void) {
	
	SocketUDP *sock = (SocketUDP *)malloc(sizeof(SocketUDP));
    initSocketUDP(sock);
    AdresseInternet *connexion =(AdresseInternet *)malloc(sizeof(AdresseInternet));
    char reponse[TFTP_SIZE];
    size_t replength;
    int block = 0;
    if (attacherSocketUDP(sock, ADDR_SERVER, TFTP_PORT, 0) != 0) {
        perror("attacherSocketUDP");
        return EXIT_FAILURE;
    }
    
    
    if (tftp_send_RRQ_wait_DATA(sock, sock->addr, FILENAME, connexion, reponse, &replength) != 1) {
        perror("tftp_send_RRQ_wait_DATA");
        return EXIT_FAILURE;
    }
    
    printf("%s\n", reponse);
    while (replength == TFTP_SIZE) {
	if ( tftp_send_ACK_wait_DATA(sock, sock->addr, block, connexion, reponse, replength) != 0) {
        perror("tftp_send_ACK_wait_DATA");
        return EXIT_FAILURE;        
    }
    printf("%s\n", reponse);
	
    }
	tftp_send_last_ACK(sock, connexion, block);
    close(sock->sockfd);
	free(sock);
}
