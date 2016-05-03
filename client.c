#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"
#include "socketUDP.h"
#include <unistd.h>

#include "tftp.h"
#define TFTP_PORT 5555
#define FILENAME "lenarraypourlesnuls.txt\0"

int main(void) {
	SocketUDP *sock = createSocketUDP();
    initSocketUDP(sock);
    AdresseInternet *server = AdresseInternet_loopback(6969);
    char reponse[TFTP_SIZE];
    size_t replength;
    int block = 0;
    if (attacherSocketUDP(sock, NULL, TFTP_PORT, INADDR_LOOPBACK) != 0) {
        fprintf(stderr, "attacherSocketUDP");
        return EXIT_FAILURE;
    }

    if (tftp_send_RRQ_wait_DATA(sock, server, FILENAME, sock->addr, reponse, &replength) != 0) {
        fprintf(stderr, "erreur tftp_send_RRQ_wait_DATA\n");
        return EXIT_FAILURE;
    }

    printf("%s\n", extract_data(reponse));
    while (replength == TFTP_SIZE) {
        if (tftp_send_ACK_wait_DATA(sock, sock->addr, block, server, reponse, replength) != 0) {
            fprintf(stderr, "erreur tftp_send_ACK_wait_DATA\n");
            return EXIT_FAILURE;        
        }
         printf("%s\n", extract_data(reponse));
	
    }

	tftp_send_last_ACK(sock, server, block);
    close(sock->sockfd);
    AdresseInternet_free(server);
	free(sock);
}
