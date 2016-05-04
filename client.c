#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"
#include "socketUDP.h"
#include <unistd.h>

#include "tftp.h"
#define FILENAME "aa3d.tar\0"
#define IPSERVER "10.130.162.72"

int main(void) {
    char nomfic[256];
    fscanf(stdin, "%s", nomfic);

	SocketUDP *sock = createSocketUDP();
    initSocketUDP(sock);
    AdresseInternet *server = AdresseInternet_new(IPSERVER, 6969);
    char reponse[TFTP_SIZE];
    size_t replength;
    int block = 0;
    if (attacherSocketUDP(sock, "0.0.0.0", 0, 0) != 0) {
        fprintf(stderr, "attacherSocketUDP");
        return EXIT_FAILURE;
    }
    int err = tftp_send_RRQ_wait_DATA(sock, server, nomfic, sock->addr, reponse, &replength);
    if (err  != 0) {
        fprintf(stderr, "erreur tftp_send_RRQ_wait_DATA\n");
        printf("%s\n", err == -2 ? "FILE_NOT_FOUND" : "Erreur lors de l'envois des données");
        return EXIT_FAILURE;
    }
    
    FILE *file = fopen("aa3d", "w");
    int transferFinished = 0;
    while (transferFinished == 0) {
        block = extract_blocknumber(reponse);
        opcode code = extract_opcode(reponse);

        // Réception des données du fichier récupéré
        if(code == DATA) {
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

    
    closeSocketUDP(sock);
    
    return EXIT_SUCCESS;
}
