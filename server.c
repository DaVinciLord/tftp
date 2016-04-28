#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"
#include "socketUDP.h"

#include "tftp.h"
#define ADDR_SERVER "10.123.123.01"
#define TFTP_PORT 6969


int main(void) {
	
	SocketUDP *sock = (SocketUDP *)malloc(sizeof(SocketUDP));
    initSocketUDP(sock);
    AdresseInternet *connexion =(AdresseInternet *)malloc(sizeof(AdresseInternet));
    char reponse[TFTP_SIZE];
    size_t replength;
    int block = 0;
    
    attacherSocketUDP(sock, ADDR_SERVER, TFTP_PORT, 0);
    
    
    tftp_wait_RRQ_send_DATA(sock, sock->addr, FILENAME, connexion, reponse, &replength); //à tester

    while (replength == TFTP_SIZE) {
	tftp_send_ACK_wait_DATA(sock, sock->addr, block, connexion, reponse, replength); //à tester
    printf("%s", reponse);
	}
	tftp_send_last_ACK(sock, connexion, block);
	free(sock);
}
