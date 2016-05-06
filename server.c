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

/**
 * 
 * Projet TFTP
 * Metton Vincent
 * Pommier Grégoire
 * 
 * 
 * */

int main(void) {
	//On créé la socket
	SocketUDP *sock = createSocketUDP();
    initSocketUDP(sock);
    //On la bind
    attacherSocketUDP(sock, NULL, TFTP_PORT, 0);
    AdresseInternet *client_addr =(AdresseInternet *)malloc(sizeof(AdresseInternet));
    char buffer[TFTP_SIZE];
    size_t filename_len;
    char filename[256];
        
	while(1) {
	
		if(tftp_wait_RRQ(sock, client_addr, buffer, filename, &filename_len) != -1) {
			
			
			// Lancement du thread
			int errnum = 0;
			//On regarde si des options ont été données en requete
			options *opts = (options *)malloc(sizeof(options));
			opts->windowsize = DEFAULT_WINDOWSIZE;
			opts->blksize = DEFAULT_BLKSIZE;
			pthread_t th;
			
			//Selon si il y a des options, en execute une routine différente
			if(extract_rrq_opt(opts, buffer) == 0) {
				thread_args_1 *arg = malloc(sizeof(*arg));
				memset(arg, 0, sizeof(*arg));
				//On créé les arguments du thread
				if(arg == NULL) {
					fprintf(stderr, "malloc()");
					exit(EXIT_FAILURE);
				}
				
				strncpy(arg->filename, filename, filename_len);
				arg->client_addr = (AdresseInternet *)malloc(sizeof(AdresseInternet));
				AdresseInternet_copy(arg->client_addr, client_addr);
				//On lance le thread
				if((errnum = pthread_create(&th, NULL, (start_routine_type) run1, arg)) != 0) {
					fprintf(stderr, "pthread_create(): %s\n", strerror(errnum));
					exit(EXIT_FAILURE);
				}
			} else {
				//De meme mais avec la seconde routine
				thread_args_2 *arg = malloc(sizeof(*arg));
				memset(arg, 0, sizeof(*arg));

				if(arg == NULL) {
					fprintf(stderr, "malloc()");
					exit(EXIT_FAILURE);
				}
				
				strncpy(arg->filename, filename, filename_len);
				arg->client_addr = (AdresseInternet *)malloc(sizeof(AdresseInternet));
				AdresseInternet_copy(arg->client_addr, client_addr);
				arg->opts = (options *)malloc(sizeof(options));
				memcpy(arg->opts, opts, sizeof(options));
				//On lance le thread
				if((errnum = pthread_create(&th, NULL, (start_routine_type) run2, arg)) != 0) {
					fprintf(stderr, "pthread_create(): %s\n", strerror(errnum));
					exit(EXIT_FAILURE);
				}
			}
			
			//On nettoie le buffer
			memset(buffer, 0, sizeof(buffer));
			memset(client_addr, 0, sizeof(*client_addr));
			memset(filename, 0, sizeof(filename));
		}  else {
			//Ce message s'affiche quand le serveur timeout
			printf("FRAME_DROP\n");
		}
	}

	closeSocketUDP(sock);
	
	return EXIT_SUCCESS;
}

void *run1(thread_args_1 *arg) {
	if (arg == NULL) {
		fprintf(stderr, "Unexpected argument value\n");
		exit(EXIT_FAILURE);
	}

	char buffer[TFTP_SIZE];
	size_t length;
	
	SocketUDP *socket = createSocketUDP();
    initSocketUDP(socket);
    //On ouvre le fichier que l'on veux lire
	FILE* file = fopen(arg->filename, "r");
	if(file != NULL) {
		size_t data = 0;
		int num_block = 1;
		char packet	[TFTP_SIZE - 4];
		do {
			//On le lit un peu
			data = fread(packet, 1, 508, file);
			tftp_make_data(buffer, &length, num_block, packet, data);
			//On envois un paquet
			tftp_send_DATA_wait_ACK(socket, arg->client_addr, num_block, buffer, length);            
			//On nettoie le Buffer
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
/* Dans ce run, on va envoyer windowsize paquets, 
 * puis attendre un ack, puis on recommence a partir
 * du numéro du block reçu
 * 
 *  */
void *run2(thread_args_2 *arg) {
	if (arg == NULL) {
		fprintf(stderr, "Unexpected argument value\n");
		exit(EXIT_FAILURE);
	}
	size_t total_size = arg->opts->blksize + 4;
	char buffer[TFTP_SIZE];
	char buffer2[total_size];
	size_t length;
	
	SocketUDP *socket = createSocketUDP();
    initSocketUDP(socket);
    if ( arg->opts->blksize < 8 || arg->opts->blksize > 65464 || arg->opts->windowsize < 1 || arg->opts->windowsize > 65535 ) {
		tftp_send_error(socket, arg->client_addr, ILLEGAL_TFTP_OPERATION, "Invalid options.");
		return NULL;
	} 
		tftp_make_oack(buffer, &length, arg->opts->blksize, arg->opts->windowsize);
		tftp_send_OACK(socket, arg->client_addr, buffer);
		uint16_t num_block = 1;
		tftp_wait_ACK(socket, arg->client_addr, &num_block);
	FILE* file = fopen(arg->filename, "r");
	if(file != NULL) {
		size_t data = 0;
		num_block = 1;
		uint16_t curr_block = 0;
		char packet[arg->opts->blksize];
		do {
			do {
				data = fread(packet, 1, arg->opts->blksize, file);
				tftp_make_data(buffer2, &length, num_block, packet, data);
				//printf("%s\n", extract_data(buffer2));
				tftp_send_DATA(socket, arg->client_addr, buffer2, length);            
				memset(buffer2, 0, sizeof(buffer2));
				num_block++;
				curr_block++;
			} while ( data >= arg->opts->blksize && curr_block < arg->opts->windowsize);

			tftp_wait_ACK(socket, arg->client_addr, &num_block);
			fseek(file, num_block*arg->opts->blksize, SEEK_SET);
			num_block++;
			curr_block = 0;
		} while(length >= arg->opts->blksize); 
		printf("\n-------------------------Envoi terminé----------------------------\n");
	} else {
		tftp_send_error(socket, arg->client_addr, FILE_NOT_FOUND, "Le fichier n'existe pas.");
	}
	
	closeSocketUDP(socket);
	
	return NULL;
}
