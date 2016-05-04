#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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
    size_t filename_len;
    char filename[255];

    tftp_wait_RRQ(sock, client_addr, buffer, filename, &filename_len);
    printf("%s\n", extract_file(buffer));

   // tftp_send_DATA_wait_ACK(sock, client_addr, 1, extract_file(buffer), filename_len + 1, sock->addr, buffer, TFTP_SIZE);

    size_t length;
    FILE* file = fopen(filename, "r");
    if(file != NULL) {
        size_t data = 0;
        int num_block = 1;
        char packet	[TFTP_SIZE - 4];
        do {
            data = fread(packet, 1, sizeof(packet), file);
            tftp_make_data(buffer, &length, num_block, packet, data);
            num_block++;
            printf("%d    %s\n", extract_blocknumber(buffer), extract_data(buffer));
            memset(buffer, 0, sizeof(buffer));
        } while(length >= TFTP_SIZE); 
    } else {
       // tftp_send_error(sock_client, client_addr, NOT_FOUND, "Le fichier n'existe pas.");
    }

    printf("RRQ reçu, fichier demandé = %s , de taille nom de fichier = %d\n", filename, (int)filename_len);

	closeSocketUDP(sock);
}
