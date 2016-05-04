#ifndef TFTP_H_
#define TFTP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>

#include "socketUDP.h"

#define TFTP_SIZE 512
#define TIMEOUT 10
#define NB_MAX_ENVOI 10
#define BINARY_TRANSFER "octet"

enum {
    RRQ = 1, WRQ = 2, DATA = 3,ACK = 4, ERROR = 5
};
typedef uint16_t opcode;

enum {
	UNDEFINED = 0, FILE_NOT_FOUND = 1, ILLEGAL_TFTP_OPERATION = 4, UNKNOWN_TID = 5
};

typedef uint16_t errcode;

int tftp_make_ack(char *buffer, size_t *length, uint16_t block);
int tftp_make_rrq(char *buffer, size_t *length, const char *file);
int tftp_make_data(char *buffer, size_t *length, uint16_t block, const char *data, size_t n);
int tftp_make_error(char *buffer, size_t *length, uint16_t error_code, const char *message);
void tftp_send_error(SocketUDP *socket, const AdresseInternet *dst, uint16_t code, const char *msg);
int tftp_send_RRQ_wait_DATA_with_timeout(SocketUDP *socket, const AdresseInternet *dst, const char *fichier, AdresseInternet *connexion, char *reponse, size_t *replength);
int tftp_send_RRQ_wait_DATA(SocketUDP *socket, const AdresseInternet *dst, const char *file, AdresseInternet *connexion, char *response, size_t *replength);
int tftp_send_DATA_wait_ACK(SocketUDP *socket, const AdresseInternet *dst, uint16_t block, const char *paquet, size_t paquetlen);
int tftp_send_ACK_wait_DATA(SocketUDP *socket, const AdresseInternet *dst, uint16_t block, AdresseInternet *connexion, char *response, size_t *replength);
int tftp_send_last_ACK(SocketUDP *socket, const AdresseInternet *dst, uint16_t block);
int tftp_wait_RRQ(SocketUDP *socket, AdresseInternet *connexion, char *buffer, char *filename, size_t *filename_len);
opcode extract_opcode(char *buffer);
int extract_blocknumber(char *buffer);
char *extract_file(char *buffer);
char *extract_mode(char *buffer, int size);
errcode extract_errcode(char *buffer);
char *extract_err_msg(char *buffer);
char *extract_data(char *buffer);

#endif // TFTP_H_
