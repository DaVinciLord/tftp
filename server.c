#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
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
    char filename[256];
    
	while(1) {
		if(tftp_wait_RRQ(sock, client_addr, buffer, filename, &filename_len) != -1) {
			
			thread_args_type *arg = malloc(sizeof(*arg));
			memset(arg, 0, sizeof(*arg));

			if(arg == NULL) {
				fprintf(stderr, "malloc()");
				exit(EXIT_FAILURE);
			}
			
			strncpy(arg->filename, filename, filename_len);
			arg->client_addr = (AdresseInternet *)malloc(sizeof(AdresseInternet));
			AdresseInternet_copy(arg->client_addr, client_addr);

			// Lancement du thread
			int errnum = 0;
			
			pthread_t th;
			if ((errnum = pthread_create(&th, NULL, (start_routine_type) run, arg)) != 0) {
				fprintf(stderr, "pthread_create(): %s\n", strerror(errnum));
				exit(EXIT_FAILURE);
			}
			memset(buffer, 0, sizeof(buffer));
			memset(client_addr, 0, sizeof(*client_addr));
			memset(filename, 0, sizeof(filename));
		}  else {
			printf("FRAME_DROP\n");
		}
	}

    printf("RRQ reçu, fichier demandé = %s , de taille nom de fichier = %d\n", filename, (int)filename_len);

	closeSocketUDP(sock);
	
	return EXIT_SUCCESS;
}

void *run(thread_args_type *arg) {
	if (arg == NULL) {
		fprintf(stderr, "Unexpected argument value\n");
		exit(EXIT_FAILURE);
	}

	char buffer[TFTP_SIZE];
	size_t length;
	
	SocketUDP *socket = createSocketUDP();
    initSocketUDP(socket);
    
	FILE* file = fopen(arg->filename, "r");
	if(file != NULL) {
		size_t data = 0;
		int num_block = 1;
		char packet	[TFTP_SIZE - 4];
		do {
			data = fread(packet, 1, 508, file);
			tftp_make_data(buffer, &length, num_block, packet, data);
			tftp_send_DATA_wait_ACK(socket, arg->client_addr, num_block, buffer, length);            
			memset(buffer, 0, sizeof(buffer));
			num_block++;
		} while(length >= TFTP_SIZE); 
		printf("\n-------------------------Envoi terminé----------------------------\n");
	} else {
		tftp_send_error(socket, arg->client_addr, FILE_NOT_FOUND, "Le fichier n'existe pas.");
	}
	
	closeSocketUDP(socket);
	
	return NULL;
}
