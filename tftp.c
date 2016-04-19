#include "tftp.h"

int tftp_make_ack(char *buffer, size_t *length, uint16_t block) {
	if (buffer == NULL || length == NULL) return -1;
	uint16_t *packet = (uint16_t *) buffer;
	*packet = htons(ACK);
	*packet + 1 = htons(block);
    *length = 4;
    return 0;
}

int tftp_make_rrq(char *buffer, size_t *length, const char *file) {
	if (buffer == NULL || length == NULL || file == NULL) return -1;
	uint16_t *packet = (uint16_t *) buffer;
	*packet = htons(RRQ);
	memcpy(buffer + 2, file, sizeof(file));
	n = 2 + sizeof(file);
	*buffer + n = '\0';
	n += 1;
	memcpy(buffer + n, TRANSFER_TYPE, sizeof(TRANSFER_TYPE));
	n += sizeof(TRANSFER_TYPE);
	*buffer + n = '\0';
    *length = n + 1;
    return 0;
}

int tftp_make_data(char *buffer, size_t *length, uint16_t block, const char *data, size_t n) {
	//buffer deja alloué;
    if (buffer == NULL || data == NULL || length == NULL) return -1;
    // Taille du fichier indiquant qu'on envoie le dernier paquet.
    if (n > TFTP_SIZE - 4) return -1;                                 
    uint16_t *packet = (uint_16 *) buffer;
    *packet = htons(DATA);
    *packet + 1 = htons(block);
    memcpy(buffer + 4, data, n); // + 4 = entete du paquet.
    *length = n + 4;
    return 0;
}

int tftp_make_error(char *buffer, size_t *length, uint16_t error_code, const char *message) {
	if (buffer == NULL || message == NULL || length == NULL) return -1;                              
    uint16_t *packet = (uint_16 *) buffer;
    *packet = htons(ERROR);
    *packet + 1 = htons(error_code);
    memcpy(buffer + 4, message, sizeof(message)); // + 4 = entete du paquet.
    n = 4 + sizeof(message);
    *buffer + n = '\0';
    *length = n + 1;
    return 0;
}

void tftp_send_error(SocketUDP *socket, const AdresseInternet *dst, uint16_t code, const char *msg) {
	char buffer[TFTP_SIZE];
	size_t length;
	tftp_make_error(buffer, &length, code, msg);
	writeToSocketUDP(socket, dst, buffer, length);
}

int tftp_send_RRQ_wait_DATA_with_timeout(SocketUDP *socket, const AdresseInternet *dst, const char *file, AdresseInternet *connexion, char *response, size_t *replength) {
    if (socket == NULL || dst == NULL || file == NULL || connexion == NULL || response == NULL || replength == NULL) return -1;
    char buffer[TFTP_SIZE]; // 512 pour TFPT_SIZE
    size_t length;
    if (tftp_make_rrq(buffer, &length, file) < 0) {
        return -1;
    }
    if (writeToSocketUDP(socket, dst, buffer, length) < 0) {
        return -1;
    }
    int n = recvFromSocketUDP(socket, response, replenght, connexion, TIMEOUT)
    if (n < 0) return -1;
    uint16_t * tmp = (uint16_t *)response;
    uint16_t r = htons(*tmp);
    if(t != DATA) {
        tftp_send_error(socket, connexion, 4, "");
    }
    return 0;
}

int tftp_send_DATA_wait_ACK(SocketUDP *socket, const AdresseInternet *dst, const char *paquet, size_t paquetlen) {
	/* if (socket == NULL || dst == NULL || paquet == NULL) return -1;
    char buffer[TFTP_SIZE]; // 512 pour TFPT_SIZE
    size_t length;
    if (tftp_make_data(buffer, length, bla, packet, paquetlen) < 0) {
        return -1;
    }
    if (writeToSocketUDP(socket, dst, buffer, length) < 0) {
        return -1;
    }
    int n = recvFromSocketUDP(socket, response, replenght, connexion, TIMEOUT)
    if (n < 0) return -1;
    uint16_t * tmp = (uint16_t *)response;
    uint16_t r = htons(*tmp);
    if(t != DATA) {
        tftp_send_error(socket, connexion, 4, "");
    }*/
    return 0;
}
