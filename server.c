#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "server.h"
#include "socketUDP.h"

#include "tftp.h"
#define TFTP_PORT 6969


int main(void) {
	
	SocketUDP *sock = createSocketUDP();
    initSocketUDP(sock);
    attacherSocketUDP(sock, NULL, TFTP_PORT, 0);
    AdresseInternet *client_addr =(AdresseInternet *)malloc(sizeof(AdresseInternet));
    char buffer[TFTP_SIZE];
    //int block = 0;
    size_t filename_len;
    char filename[255];
    tftp_wait_RRQ(sock, client_addr, buffer, filename, &filename_len); 
    printf("RRQ reçu, fichier demandé = %s , de taille nom de fichier = %d\n", filename, (int)filename_len);

	free(sock);
}
