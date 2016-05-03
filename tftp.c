#include "tftp.h"
#include "socketUDP.h"

int tftp_make_ack(char *buffer, size_t *length, uint16_t block) {
	if (buffer == NULL || length == NULL) return -1;
	uint16_t *packet = (uint16_t *) buffer;
	*(packet) = htons(ACK);
	*(packet + 1) = htons(block);
    *length = (sizeof(uint16_t) + sizeof(block));
    return 0;
}

int tftp_make_rrq(char *buffer, size_t *length, const char *file) {
	if (buffer == NULL || length == NULL || file == NULL) return -1;
	uint16_t *packet = (uint16_t *) buffer;
	*packet = htons(RRQ);
	memcpy(buffer + 2, file, strlen(file));
	int n = 2 + sizeof(file);
	*(buffer + n) = '\0';
	n += 1;
	memcpy(buffer + n, TRANSFER_TYPE, sizeof(TRANSFER_TYPE));
	n += sizeof(TRANSFER_TYPE);
	*(buffer + n) = '\0';
    *length = n + 1;
    return 0;
}


int tftp_make_data(char *buffer, size_t *length, uint16_t block, const char *data, size_t n) {
	//buffer deja alloué;
    if (buffer == NULL || data == NULL || length == NULL) return -1;
    // Taille du fichier indiquant qu'on envoie le dernier paquet.
    if (n > TFTP_SIZE - 4) return -1;                                 
    uint16_t *packet = (uint16_t *) buffer;
    *packet = htons(DATA);
    *(packet + 1) = htons(block);
    memcpy(buffer + 4, data, n); // + 4 = entete du paquet.
    *length = n + 4;
    return 0;
}

int tftp_make_error(char *buffer, size_t *length, uint16_t error_code, const char *message) {
	if (buffer == NULL || message == NULL || length == NULL) return -1;                              
    uint16_t *packet = (uint16_t *) buffer;
    *packet = htons(ERROR);
    *(packet + 1) = htons(error_code);
    memcpy(buffer + 4, message, strlen(message)); // + 4 = entete du paquet.
    int n = 4 + strlen(message);
    *(buffer + n) = '\0';
    *length = n + 1;
    return 0;
}

opcode extract_type(char *buffer) {
	uint16_t *tmp = (uint16_t *) buffer;
	opcode type = ntohs(*tmp);
	return type;
}

int extract_blocknumber(char *buffer) {
	uint16_t *tmp = (uint16_t *) buffer;
	uint16_t block = ntohs(*(tmp + 1));
	return block;
}

char *extract_file(char *buffer) {
	return buffer + 2;
}

char *extract_mode(char *buffer, int size) {
	return buffer + size;
}

errcode extract_errcode(char *buffer) {
	uint16_t *tmp = (uint16_t *) buffer;
	errcode err_code = ntohs(*(tmp + 1));
	return err_code;
}

char *extract_err_msg(char *buffer) {
	return buffer + 4;
}

char *extract_data(char *buffer) {
	return buffer + 4;
}


void tftp_send_error(SocketUDP *socket, const AdresseInternet *dst, uint16_t code, const char *msg) {
	if(socket != NULL && dst != NULL && msg != NULL) {
        char buffer[TFTP_SIZE];
        size_t length;
        tftp_make_error(buffer, &length, code, msg);
        writeToSocketUDP(socket, dst, buffer, length);
    }
}

int tftp_wait_RRQ(SocketUDP *socket, AdresseInternet *connexion, char *buffer, char *filename, size_t *filename_len) {
    recvFromSocketUDP(socket, buffer, TFTP_SIZE, connexion, TIMEOUT);
    if (extract_type(buffer) == RRQ) {
        char *tmp = strdup(buffer);
        strcpy(filename, extract_file(tmp));
        *filename_len = strlen(filename);
        return 0;
    }
    return -1;
}


int tftp_send_RRQ_wait_DATA_with_timeout(SocketUDP *socket, const AdresseInternet *dst, const char *file, AdresseInternet *connexion, char *response, size_t *replength) {
	if (socket == NULL || dst == NULL || file == NULL || connexion == NULL || response == NULL || replength == NULL) return -1;
    char buffer[TFTP_SIZE]; // 512 pour TFPT_SIZE
    size_t length;
    if (tftp_make_rrq(buffer, &length, file) < 0) {
        return -1;
    }
    printf("%s\n", extract_file(buffer));
    if (writeToSocketUDP(socket, dst, buffer, length) < 0) {
        return -1;
    }
    ssize_t n = recvFromSocketUDP(socket, response, TFTP_SIZE, connexion, TIMEOUT);
    if (n < 0) return -1;
    *replength = n;
    uint16_t *tmp = (uint16_t *)response;
    uint16_t r = htons(*tmp);
    if(r != DATA) {
        tftp_send_error(socket, connexion, 4, "Le paquet n'est pas de type DATA.");
        return -1;
    }
    return 0;
}

int tftp_send_RRQ_wait_DATA(SocketUDP *socket, const AdresseInternet *dst, const char *file, AdresseInternet *connexion, char *response, size_t *replength) {
	for (int i = 0; i < NB_MAX_ENVOI; i++) {
		if (tftp_send_RRQ_wait_DATA_with_timeout(socket, dst, file, connexion, response, replength) != -1) {
            return 0;
        }
	}
	return -1;
}

int tftp_send_DATA_wait_ACK(SocketUDP *socket, const AdresseInternet *dst, uint16_t block, const char *paquet, size_t paquetlen, AdresseInternet *connexion, char *response, size_t replength) {
	if (socket == NULL || dst == NULL || paquet == NULL) return -1;
    char buffer[TFTP_SIZE]; // 512 pour TFPT_SIZE
    size_t length;
    if (tftp_make_data(buffer, &length, block, paquet, paquetlen) < 0) {
        return -1;
    }
    if (writeToSocketUDP(socket, dst, buffer, length) < 0) {
        return -1;
    }
    int n = recvFromSocketUDP(socket, response, replength, connexion, TIMEOUT);
    if (n < 0) return -1;
    uint16_t * tmp = (uint16_t *)response;
    uint16_t r = htons(*tmp);
    if(r != ACK) {
        tftp_send_error(socket, connexion, 4, "");
    }
    return 0;
}

int tftp_send_ACK_wait_DATA(SocketUDP *socket, const AdresseInternet *dst, uint16_t block, AdresseInternet *connexion, char *response, size_t replength) {
	if (socket == NULL || dst == NULL) return -1;
    char buffer[TFTP_SIZE]; // 512 pour TFPT_SIZE
    size_t length;
    if (tftp_make_ack(buffer, &length, block) < 0) {
        return -1;
    }
    if (writeToSocketUDP(socket, dst, buffer, length) < 0) {
        return -1;
    }
    int n = recvFromSocketUDP(socket, response, replength, connexion, TIMEOUT);
    if (n < 0) return -1;
    uint16_t * tmp = (uint16_t *)response;
    uint16_t r = htons(*tmp);
    if(r != DATA) {
        tftp_send_error(socket, connexion, 4, "");
    }
    return 0;
}

int tftp_send_last_ACK(SocketUDP *socket, const AdresseInternet *dst, uint16_t block) {
	if (socket == NULL || dst == NULL) return -1;
    char buffer[TFTP_SIZE]; // 512 pour TFPT_SIZE
    size_t length;
    if (tftp_make_ack(buffer, &length, block) < 0) {
        return -1;
    }
    if (writeToSocketUDP(socket, dst, buffer, length) < 0) {
        return -1;
    }
    return 0;
}

