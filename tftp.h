#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#define TFTP_SIZE 512
#define TIMEOUT 42
#define TRANSFER_TYPE "octet"
enum opcode {
    RRQ = 1, WRQ = 2, DATA = 3,ACK = 4, ERROR = 5
};

enum errorcode {
	UNDEFINED = 0, FILE_NOT_FOUND = 1, ILLEGAL_TFTP_OPERATION = 4, UNKNOWN_TID = 5
};

typedef enum opcode opcode;
typedef enum errorcode errorcode;

int tftp_make_ack(char *buffer, size_t *length, uint16_t block);
int tftp_make_rrq(char *buffer, size_t *length, const char *file);
int tftp_make_data(char *buffer, size_t *length, uint16_t block, const char *data, size_t n);
int tftp_make_error(char *buffer, size_t *length, uint16_t error_code, const char *message);
void tftp_send_error(SocketUDP *socket, const AdresseInternet *dst, uint16_t code, const char *msg);
int tftp_send_RRQ_wait_DATA_with_timeout(SocketUDP *socket, const AdresseInternet *dst, const char *fichier, AdresseInternet *connexion, char *reponse, size_t *replength);
int tftp_send_DATA_wait_ACK(SocketUDP *socket,const AdresseInternet *dst, const char *paquet, size_t paquetlen);
