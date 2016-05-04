#include "tftp.h"
#include "options.h"
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

int write_options(char *buffer, const char *option, int value) {
	strncpy(buffer, option, strlen(option) + 1);
    int n = strlen(option) + 1;
    n += snprintf((buffer + n), sizeof(value), "%d", value);
    *(buffer + n) = '\0';
    n += 1;
    return n;
}

int tftp_make_rrq_opt(char *buffer, size_t *length, const char *fichier, size_t noctets, size_t nblocs) {
	if (buffer == NULL || length == NULL || fichier == NULL) return -1;
	
	tftp_make_rrq(buffer, length, fichier);
	int n = *length;
	
	n += write_options(buffer + n, "windowsize", noctets);
	n += write_options(buffer + n, "blksize", nblocs);
	
	*length = n;
    return 0;
}

int tftp_make_oack(char *buffer, size_t *length, size_t noctets, size_t nblocs) {
	if (buffer == NULL || length == NULL) return -1;
	
	uint16_t *packet = (uint16_t *) buffer;
	*packet = htons(OACK);
	int n = 2;
    n += write_options(buffer + n, "windowsize", noctets);
    n += write_options(buffer + n, "blksize", nblocs);
    *length = n;
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

int extract_rrq_opt(options *opts, char *buffer) {
    int n = strlen(buffer) + 1;
    printf("%s\n", buffer + n);
	if(strcmp("windowsize", buffer + n) != 0) {
		return 0;
	} 
    n += strlen("windowsize") + 1;
    opts->windowsize = atoi(buffer + n);
    n += strlen(buffer + n) + 1;
    n += strlen("blksize") + 1;
    opts->blksize = atoi(buffer + n);
    n += strlen(buffer + n) + 1;
    
    return 1;
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
        *replength = n;
		if(r != DATA) {
			tftp_send_error(socket, dst, ERROR, "C'est pas légal!\n");
			return -1;
		}
		return 0;
	}
    return -1;
}

int tftp_send_RRQ_wait_DATA(SocketUDP *socket, const AdresseInternet *dst, const char *file, AdresseInternet *connexion, char *response, size_t *replength) {
	for (int i = 0; i < NB_MAX_ENVOI; i++) {
		if (tftp_send_RRQ_wait_DATA_with_timeout(socket, dst, file, connexion, response, replength) != -1) {
            return extract_opcode(response) == ERROR ? -extract_errcode(response) : 0;
        }
	}
	return -1;
}

int tftp_send_DATA_wait_ACK(SocketUDP *socket, const AdresseInternet *dst, uint16_t block, const char *paquet, size_t paquetlen) {
	if (socket == NULL || dst == NULL || paquet == NULL) return -1;
    char response[TFTP_SIZE];
    do {
        if (writeToSocketUDP(socket, dst, paquet, paquetlen) < 0) {
            return -1;
        }
        AdresseInternet add_tmp;
        memset(&add_tmp, 0, sizeof(add_tmp));
        int n = recvFromSocketUDP(socket, response, TFTP_SIZE, &add_tmp, TIMEOUT);
        if (n < 0) return -1;
        opcode r = extract_opcode(response);
        if(r != ACK) {
            tftp_send_error(socket, dst, 4, "");
        }
    } while (extract_blocknumber(response) != block);
    return 0; 
}

int tftp_send_ACK_wait_DATA(SocketUDP *socket, const AdresseInternet *dst, uint16_t block, AdresseInternet *connexion, char *response, size_t *replength) {
	if (socket == NULL || dst == NULL) return -1;
    char buffer[TFTP_SIZE]; // 512 pour TFPT_SIZE
    size_t length;
    if (tftp_make_ack(buffer, &length, block) < 0) {
        return -1;
    }
    if (writeToSocketUDP(socket, dst, buffer, TFTP_SIZE) < 0) {
        return -1;
    }
    int n = recvFromSocketUDP(socket, response, TFTP_SIZE, connexion, TIMEOUT);
    if (n < 0) return -1;
    opcode r = extract_opcode(response);
    if(r != DATA) {
        tftp_send_error(socket, dst, 4, "");
    }
    *replength = n;
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

int tftp_send_ACK(SocketUDP *socket, const AdresseInternet *dst, uint16_t block) {
	return tftp_send_last_ACK(socket, dst, block);
}

int tftp_wait_DATA(SocketUDP *socket, AdresseInternet *connexion, char *response, size_t *replength) {
	if (socket == NULL) return -1;
	int n = recvFromSocketUDP(socket, response, TFTP_SIZE, connexion, TIMEOUT);
    if (n < 0) return -1;
    opcode r = extract_opcode(response);
    if(r != DATA) {
        tftp_send_error(socket, connexion, 4, "");
    }
    *replength = n;
    return 0;
}

int tftp_send_RRQ_wait_OACK_with_timeout(SocketUDP *socket, const AdresseInternet *dst, const char *filename, AdresseInternet *connexion, char *response, size_t *replength, size_t noctets, size_t nblocs) {
	char buffer[TFTP_SIZE]; 
    size_t length;
    if (tftp_make_rrq_opt(buffer, &length, filename, noctets, nblocs) < 0) {
        return -1;
    }
    if (writeToSocketUDP(socket, dst, buffer, length) < 0) {
        return -1;
    }
    int n  = recvFromSocketUDP(socket, response, TFTP_SIZE, connexion, TIMEOUT);
    if (n < 0) return -1;
    if (n != 0) { 
		opcode r = extract_opcode(response);    
        *replength = n;
		if(r != OACK) {
			tftp_send_error(socket, dst, ERROR, "C'est pas légal!\n");
			printf("%d\n", (int) r);
			return -1;
		}
		return 0;
	}
    return -1;
}


int tftp_send_RRQ_wait_OACK(SocketUDP *socket, const AdresseInternet *dst, const char *filename, AdresseInternet *connexion, char *response, size_t *replength, size_t noctets, size_t nblocs) {
	for (int i = 0; i < NB_MAX_ENVOI; i++) {
		if (tftp_send_RRQ_wait_OACK_with_timeout(socket, dst, filename, connexion, response, replength, noctets, nblocs) != -1) {
            return extract_opcode(response) == ERROR ? -extract_errcode(response) : 0;
        }
	}
	return -1;
}

int tftp_send_DATA(SocketUDP *socket, const AdresseInternet *dst, const char *paquet, size_t paquetlen) {
	if (socket == NULL || dst == NULL || paquet == NULL) return -1;
        if (writeToSocketUDP(socket, dst, paquet, paquetlen) < 0) {
            return -1;
        }
    
    return 0; 
}

int tftp_wait_ACK(SocketUDP *socket, const AdresseInternet *dst, uint16_t *block) {
	if (socket == NULL || dst == NULL) return -1;
    char response[TFTP_SIZE];

        AdresseInternet add_tmp;
        memset(&add_tmp, 0, sizeof(add_tmp));
        int n = recvFromSocketUDP(socket, response, TFTP_SIZE, &add_tmp, TIMEOUT);
        if (n < 0) return -1;
        opcode r = extract_opcode(response);
        if(r != ACK) {
            tftp_send_error(socket, dst, 4, "");
        }
        *block = extract_blocknumber(response);
    return 0; 
}

int tftp_send_OACK(SocketUDP *socket, const AdresseInternet *dst, const char *buffer) {
	if (socket == NULL || dst == NULL) return -1;
    if (writeToSocketUDP(socket, dst, buffer, TFTP_SIZE) < 0) {
        return -1;
    }
    return 0;
}
