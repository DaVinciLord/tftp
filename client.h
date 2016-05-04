#ifndef CLIENT_H_
#define CLIENT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "socketUDP.h"
#include "tftp.h"

#define TFTP_PORT 6969
int trivial_tftp(int block, size_t replength, char *reponse, AdresseInternet *server, size_t blk_size, SocketUDP *sock, char *nomfic);
int hard_tftp(int block, size_t replength, char *reponse, AdresseInternet *server, size_t blk_size, SocketUDP *sock, char *nomfic, size_t window_size);


#endif // CLIENT_H_
