#include "tftp.h"
#include "socketUDP.h"

int tftp_make_ack(char *buffer, size_t *length, uint16_t block) {
	if (buffer == NULL || length == NULL) return -1;
	uint16_t *packet = (uint16_t *) buffer;
	*(packet) = htons(ACK);
	*(packet + 1) = htons(block);
    *length = 4;
    return 0;
}

int tftp_make_rrq(char *buffer, size_t *length, const char *file) {
	if (buffer == NULL || length == NULL || file == NULL) return -1;
	uint16_t *packet = (uint16_t *) buffer;
	*packet = htons(RRQ);
	int n = 2;
	strncpy(buffer + 2, file, strlen(file) + 1);
	n +=  strlen(file) + 1;
	strncpy(buffer + n, BINARY_TRANSFER, strlen(BINARY_TRANSFER) + 1);
	n += strlen(BINARY_TRANSFER) + 1;
	*(buffer + n) = '\0';
	*length = n;
    return 0;
}


int tftp_make_data(char *buffer, size_t *length, uint16_t block, const char *data, size_t n) {
	//buffer deja alloué;
    if (buffer == NULL || data == NULL || length == NULL) return -1;
    // Taille du fichier indiquant qu'on envoie le dernier paquet.
    if (n > TFTP_SIZE - 4) return -1;                           
    uint16_t *packet = (uint16_t *) buffer;
    *packet = htons(DATA);
    int i = 2;
    *(packet + 1) = htons(block);
    i += 2;
    memcpy(buffer + i, data, n);
    *length = n + i;
    return 0;
}

int tftp_make_error(char *buffer, size_t *length, uint16_t error_code, const char *message) {
	if (buffer == NULL || message == NULL || length == NULL) return -1;                              
    uint16_t *packet = (uint16_t *) buffer;
    *packet = htons(ERROR);
	int n =2;
    *(packet + 1) = htons(error_code);
    n += 2;
    strncpy(buffer + n, message, strlen(message));
    n += strlen(message);
    *(buffer + n) = '\0';
    *length = n + 1;
    return 0;
}

opcode extract_opcode(char *buffer) {
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
    if (extract_opcode(buffer) == RRQ) {
        strcpy(filename, extract_file(buffer));
        *filename_len = strlen(filename);
        return 0;
    }
    return -1;
}



int tftp_send_RRQ_wait_DATA_with_timeout(SocketUDP *socket, const AdresseInternet *dst, const char *file, AdresseInternet *connexion, char *response, size_t *replength) {
	char buffer[TFTP_SIZE]; 
    size_t length;
    if (tftp_make_rrq(buffer, &length, file) < 0) {
        return -1;
    }
    if (writeToSocketUDP(socket, dst, buffer, length) < 0) {
        return -1;
    }
    int n  = recvFromSocketUDP(socket, response, TFTP_SIZE, connexion, TIMEOUT);
    if (n < 0) return -1;
    if (n != 0) { 
		opcode r = extract_opcode(response);
		if(r != DATA) {
			tftp_send_error(socket, dst, ERROR, "C'est pas légal!\n");
			return -1;
		}
		return 0;
	}
    *replength = n;
    return -1;
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

