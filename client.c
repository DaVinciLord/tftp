#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"
#include "socketUDP.h"
#include <unistd.h>

#include "tftp.h"
#define FILENAME "aa3d.tar\0"
#define IPSERVER "10.130.162.72"
int trivial_tftp(int block, size_t replength, char *reponse, AdresseInternet *server, size_t blk_size, SocketUDP *sock, char *nomfic);
int hard_tftp(int block, size_t replength, char *reponse, AdresseInternet *server, size_t blk_size, SocketUDP *sock, char *nomfic, size_t window_size);

int main(void) {
    char nomfic[256];
    size_t blk_size = TFTP_SIZE;
    size_t window_size = 1;
    char option[10];
    printf("Entrez le nom du fichier à importer : ");
    fscanf(stdin, "%s", nomfic);
    printf("Voulez vous des options ? ");
    fscanf(stdin, "%s", option);    
    if (strcmp(option, "O") == 0 || strcmp(option, "o") == 0 || strcmp(option, "Oui") == 0) {
        printf("\n Taille des paquets ? > 8 et < 65464 :");
        fscanf(stdin, "%zu", &blk_size);
        printf("\n Taille des fenêtres ? > 1 et < 65535 :");
        fscanf(stdin, "%zu", &window_size);
        printf("\n");
    }   
    
	SocketUDP *sock = createSocketUDP();
    initSocketUDP(sock);
    AdresseInternet *server = AdresseInternet_new(IPSERVER, 6969);
    char reponse[TFTP_SIZE];
    size_t replength = blk_size;
    int block = 0;
    if (attacherSocketUDP(sock, "0.0.0.0", 0, 0) != 0) {
        fprintf(stderr, "attacherSocketUDP");
        return EXIT_FAILURE;
    }
    if (blk_size == TFTP_SIZE &&  window_size == 1) {
        trivial_tftp(block, replength, reponse, server, blk_size, sock, nomfic);
    } else {
        hard_tftp(block, replength, reponse, server, blk_size, sock, nomfic, window_size );

    }
    
    closeSocketUDP(sock);
    
    return EXIT_SUCCESS;
}


int trivial_tftp(int block, size_t replength, char *reponse, AdresseInternet *server, size_t blk_size, SocketUDP *sock, char *nomfic) {
    int err = tftp_send_RRQ_wait_DATA(sock, server, nomfic, sock->addr, reponse, &replength);

    if (err  != 0) {
        fprintf(stderr, "erreur tftp_send_RRQ_wait_DATA\n");
        printf("%s\n", err != -1 ? extract_err_msg(reponse) : "Erreur lors de l'envois des données");
        return EXIT_FAILURE;
    }

    
    FILE *file = fopen("aa3d", "w");
    int transferFinished = 0;
    while (transferFinished == 0) {
        block = extract_blocknumber(reponse);
        opcode code = extract_opcode(reponse);
        printf("%d\n", (int) extract_blocknumber(reponse));
        // Réception des données du fichier récupéré
        if(code == DATA) {
            fwrite(extract_data(reponse), replength-4, 1, file);
            if (replength < blk_size) {
                tftp_send_last_ACK(sock, server , block);
                printf("Transfert terminé\n");
                transferFinished = 1;
            } else {
                memset(reponse, 0, TFTP_SIZE);
                replength = blk_size;
                tftp_send_ACK_wait_DATA(sock, sock->addr, block, server, reponse, &replength);
            }
        }
    }
    return EXIT_SUCCESS;
}
    
int hard_tftp(int block, size_t replength, char *reponse, AdresseInternet *server, size_t blk_size, SocketUDP *sock, char *nomfic, size_t window_size ) {
    int err = tftp_send_RRQ_wait_OACK(sock, server, nomfic, sock->addr, reponse, &replength, blk_size, window_size);

    if (err  != 0) {
        fprintf(stderr, "erreur tftp_send_RRQ_wait_DATA\n");
        printf("%s\n", err != -1 ? extract_err_msg(reponse) : "Erreur lors de l'envois des données");
        return EXIT_FAILURE;
    }
     
    tftp_send_ACK(sock, server, 0);

    
    FILE *file = fopen("aa3d", "w");
    int transferFinished = 0;
    size_t lastblockasked = 1;
    while (transferFinished == 0) {
        
        tftp_wait_DATA(sock, sock->addr, reponse);
        
        block += extract_blocknumber(reponse) == block + 1 ? 1 : 0;
        
        opcode code = extract_opcode(reponse);
        printf("%d\n", (int) extract_blocknumber(reponse));
        
        // Réception des données du fichier récupéré
        if(code == DATA && block == extract_blocknumber(reponse)) {
            fwrite(extract_data(reponse), replength-4, 1, file);
            if (replength < blk_size) {
                tftp_send_last_ACK(sock, server , block);
                printf("Transfert terminé\n");
                transferFinished = 1;
            } else {
                memset(reponse, 0, TFTP_SIZE);
                replength = blk_size;
                if (lastblockasked == window_size)  {
                    tftp_send_ACK(sock, sock->addr, block);
                    lastblockasked = 1;
                }
            }
        }
    }
    return EXIT_SUCCESS;
}
