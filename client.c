#include "client.h"

/**
 * 
 * Projet TFTP
 * Metton Vincent
 * Pommier Grégoire
 * 
 * 
 * */



int main(void) {
   
	//On initialise les variables qui seront entrées par l'utilisateur
    char nomfic[256];
    size_t blk_size = TFTP_SIZE;
    size_t window_size = 1;
    char option[10];
    char ipserver[128];
    
    printf("Entrez l'IP XXX.XXX.XXX.XXX: ");
    fscanf(stdin, "%s", ipserver);
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
    
    //On créé la socket
	SocketUDP *sock = createSocketUDP();
    initSocketUDP(sock);
    //On défini l'adresse du serveur
    AdresseInternet *server = AdresseInternet_new(ipserver, 6969);
    
    size_t replength = blk_size;
    int block = 0;
    
    // On bind la socket
    if (attacherSocketUDP(sock, "0.0.0.0", 0, 0) != 0) {
        fprintf(stderr, "attacherSocketUDP");
        return EXIT_FAILURE;
    }
    
    // Selon si des options ont été demandées on choisi ce que l'on envoit
    if (blk_size == TFTP_SIZE &&  window_size == 1) {
		char reponse[TFTP_SIZE];
        trivial_tftp(block, replength, reponse, server, blk_size, sock, nomfic);
    } else {
		char reponse[blk_size + 4];
        hard_tftp(block, replength, reponse, server, blk_size, sock, nomfic, window_size );
    }
    
    closeSocketUDP(sock);
    
    return EXIT_SUCCESS;
}

//C'est la fonction classique, qui ne gère pas les options windowsize et blocksize
int trivial_tftp(int block, size_t replength, char *reponse, AdresseInternet *server, size_t blk_size, SocketUDP *sock, char *nomfic) {
    //On envoit la requete 
    int err = tftp_send_RRQ_wait_DATA(sock, server, nomfic, sock->addr, reponse, &replength);
    if (err  != 0) {
        fprintf(stderr, "erreur tftp_send_RRQ_wait_DATA\n");
        printf("%s\n", err != -1 ? extract_err_msg(reponse) : "Erreur lors de l'envois des données");
        return EXIT_FAILURE;
    }
    
	//On initialise le chronomètre
    time_t t = time(NULL);
    
    //On ouvre le fichier de sortie
    char newname[256] = "./out/";
    strncat(newname, nomfic, 256);
    FILE *file = fopen(newname, "w");
    int transferFinished = 0;
    
    //Tant qu'on l'on ne reçois pas le paquet final
    while (transferFinished == 0) {
        block = extract_blocknumber(reponse);
        opcode code = extract_opcode(reponse);
        // Réception des données du fichier récupéré
        if(code == DATA) {
            fwrite(extract_data(reponse), replength-4, 1, file);
            if (replength < blk_size) {
				// On envois le dernier paquet d'aquitement
                tftp_send_last_ACK(sock, server , block);
                printf("Transfert terminé\n");
                transferFinished = 1;
            } else {
                memset(reponse, 0, TFTP_SIZE);
                replength = blk_size;
                // On envois le paquet d'aquitement du bloc reçu
                tftp_send_ACK_wait_DATA(sock, sock->addr, block, server, reponse, &replength);
                
            }
        }
    }
    printf("%d secondes\n" , (int) (time(NULL) - t)); 
    return EXIT_SUCCESS;
}
    
int hard_tftp(int block, size_t replength, char *reponse, AdresseInternet *server, size_t blk_size, SocketUDP *sock, char *nomfic, size_t window_size ) {
	//On envois la requete avec les options
    int err = tftp_send_RRQ_wait_OACK(sock, server, nomfic, sock->addr, reponse, &replength, blk_size, window_size);

    if (err  != 0) {
        fprintf(stderr, "erreur tftp_send_RRQ_wait_OACK\n");
        printf("%s\n", err != -1 ? extract_err_msg(reponse) : "Erreur lors de l'envois des données\n");
        return EXIT_FAILURE;
    }
     
    tftp_send_ACK(sock, server, 0);

    // On ouvre le fichier
    FILE *file = fopen("aa3d", "w");
    int transferFinished = 0;
    size_t lastblockasked = 1;
    while (transferFinished == 0) {
        //On reçois des données
        tftp_wait_DATA(sock, sock->addr, reponse, &replength);
        
        block += extract_blocknumber(reponse) == block + 1 ? 1 : 0;
        
        opcode code = extract_opcode(reponse);
        
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
					// Au bout de windowsize, on envois l'ack du dernier block correct reçu
                    tftp_send_ACK(sock, sock->addr, block);
                    lastblockasked = 1;
                }
            }
        }
    }
    return EXIT_SUCCESS;
}
