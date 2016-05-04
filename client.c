#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"
#include "socketUDP.h"
#include <unistd.h>

#include "tftp.h"
#define TFTP_PORT 5555
#define FILENAME "lenarraypourlesnuls.txt\0"
#define IPSERVER "10.130.162.72"

int main(void) {
	SocketUDP *sock = createSocketUDP();
    initSocketUDP(sock);
    AdresseInternet *server = AdresseInternet_new(IPSERVER, 6969);
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
    FILE *file = fopen("out.txt", "w");
    int transferFinished = 0;
    while (transferFinished == 0) {
        block = extract_blocknumber(reponse);
        opcode code = extract_opcode(reponse);

        // Réception des données du fichier récupéré
        if(code == DATA) {
            printf("DATA n° %d\n", extract_blocknumber(reponse));
            printf("%d\n", (int)replength);
            fwrite(extract_data(reponse), replength-4, 1, file);
            if (replength < TFTP_SIZE) {
                tftp_send_last_ACK(sock, server , block);
                printf("Transfert terminé\n");
                transferFinished = 1;
            } else {
                memset(reponse, 0, sizeof(reponse));
                tftp_send_ACK_wait_DATA(sock, sock->addr, block, server, reponse, &replength);
            }
        }
    }

    
    close(sock->sockfd);
    AdresseInternet_free(server);
	free(sock);
}
