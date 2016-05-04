#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "AdresseInternetType.h"

AdresseInternet *AdresseInternet_new (const char* adresse, uint16_t port) {
    /*Allocation mémoire pour la structure */
    AdresseInternet *addr = (AdresseInternet*)malloc(sizeof(*addr));
    /* Déclaration des structures à utiliser avec getaddrinfo */
    struct addrinfo hints, *res;
    char service[BUFSIZ];
    int err;
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags |= AI_CANONNAME;
    sprintf(service, "%d", port); 
    if((err = getaddrinfo(adresse, service, &hints, &res)) != 0) {
        printf("%s", gai_strerror(err));
        exit(EXIT_FAILURE);
    }
    /* Si l'appel a réussi on récupere la première adresse disponible */
    if(res->ai_family == AF_INET) {
        struct sockaddr_in *tmp = (struct sockaddr_in*)&(addr->sockAddr);
        memcpy(tmp, res->ai_addr, sizeof(struct sockaddr_in));
    } else if (res->ai_family == AF_INET6) {
        struct sockaddr_in6 *tmp = (struct sockaddr_in6*)&(addr->sockAddr);
        memcpy(tmp, res->ai_addr, sizeof(struct sockaddr_in6));
    }
    strcpy(addr->nom, res->ai_canonname);
    strcpy(addr->service, service);
    freeaddrinfo(res);
    return addr;
}

AdresseInternet *AdresseInternet_any (uint16_t port) {
    return AdresseInternet_new("0.0.0.0", port);
}

AdresseInternet *AdresseInternet_loopback (uint16_t port) {
    return AdresseInternet_new("127.0.0.1", port);
}

void AdresseInternet_free (AdresseInternet *adresse) {
    if(adresse != NULL) {
        free(adresse);
    }
}

int AdresseInternet_getinfo (AdresseInternet *adresse, char *nomDNS, int tailleDNS, char *nomPort, int taillePort) {
    if(adresse == NULL || (nomDNS == NULL && nomPort == NULL)) {
        return -1;
    }
    if(nomDNS != NULL) {
        if(!(strlen(adresse->nom) > (unsigned)tailleDNS)) {
            strncpy(nomDNS, adresse->nom, strlen(adresse->nom+1));
        } else {
            return -1;
        }
    }
    if(nomPort != NULL) {
        if(!(strlen(adresse->service) > (unsigned)taillePort)) {
            strncpy(nomPort, adresse->service, strlen(adresse->service+1));
        } else {
            return -1;
        }
    }
    return 0;
}

int AdresseInternet_getIP (const AdresseInternet *adresse, char *IP, int tailleIP) {
    if(adresse == NULL) {
        return -1;
    }
    if(adresse->sockAddr.ss_family == AF_INET) {
       struct sockaddr_in *tmp = (struct sockaddr_in*)(&adresse->sockAddr);
       if(inet_ntop(AF_INET, (void*)(&tmp->sin_addr), IP, tailleIP) == NULL) {
           return -1;
       }
    } else if (adresse->sockAddr.ss_family == AF_INET6) {
       struct sockaddr_in6* tmp = (struct sockaddr_in6*)(&adresse->sockAddr);
       if(inet_ntop(AF_INET6, (void*)(&tmp->sin6_addr), IP, tailleIP) == NULL) {
           return -1;
       }
    }
    return 0;
}

uint16_t AdresseInternet_getPort (const AdresseInternet *adresse) {
    if(adresse == NULL) {
        return -1;
    }
    uint16_t port = 0;
    port = atoi(adresse->service);
    return port;
}


int sockaddr_to_AdresseInternet (const struct sockaddr *addr, AdresseInternet *adresse) {
    if(addr == NULL || adresse == NULL ) {
        return -1;
    }
    if(addr->sa_family == AF_INET) {
        struct sockaddr_in* tmp = (struct sockaddr_in*)addr;
        memcpy(&adresse->sockAddr, addr, sizeof(*tmp));
        sprintf(adresse->nom, "%d", (int)tmp->sin_addr.s_addr);
        sprintf(adresse->service, "%hu", tmp->sin_port);
    } else if (addr->sa_family == AF_INET6) {
        struct sockaddr_in6* tmp = (struct sockaddr_in6*)addr;
        memcpy(&adresse->sockAddr, addr, sizeof(*tmp));
        sprintf(adresse->nom, "%s", tmp->sin6_addr.s6_addr);
        sprintf(adresse->service, "%hu", tmp->sin6_port);
    }
    return 0;
}

int AdresseInternet_to_sockaddr (const AdresseInternet *adresse, struct sockaddr *addr) {
    if(adresse == NULL || addr == NULL) {
        return -1;
    }
    if(adresse->sockAddr.ss_family == AF_INET) {
        memcpy(addr, (struct sockaddr_in*)&adresse->sockAddr, sizeof(struct sockaddr_in));
    } else if(adresse->sockAddr.ss_family == AF_INET6) {
        memcpy(addr, (struct sockaddr_in6*)&adresse->sockAddr, sizeof(struct sockaddr_in6));
    }
    return 0;
}

int AdresseInternet_compare (const AdresseInternet *adresse1, const AdresseInternet *adresse2) {
    if(adresse1 == NULL || adresse2 == NULL) {
        return -1;
    }
    if(adresse1->sockAddr.ss_family != adresse2->sockAddr.ss_family) {
        return 0;
    }
    if(adresse1->sockAddr.ss_family == AF_INET) {
        struct sockaddr_in* tmp1 = (struct sockaddr_in*)&adresse1->sockAddr;
        struct sockaddr_in* tmp2 = (struct sockaddr_in*)&adresse2->sockAddr;
        if(tmp1->sin_addr.s_addr == tmp2->sin_addr.s_addr && tmp1->sin_port == tmp2->sin_port) {
            return 1;
        }
    } else if(adresse1->sockAddr.ss_family == AF_INET6) {
        struct sockaddr_in6* tmp1 = (struct sockaddr_in6*)&adresse1->sockAddr;
        struct sockaddr_in6* tmp2 = (struct sockaddr_in6*)&adresse2->sockAddr;
        if(memcmp(&tmp1->sin6_addr, &tmp2->sin6_addr, sizeof(struct in6_addr)) && tmp1->sin6_port == tmp2->sin6_port) {
            return 1;
        }
    }
    return 0;
}

int AdresseInternet_copy (AdresseInternet *adrdst, const AdresseInternet *adrsrc) {
    memcpy(adrdst, adrsrc, sizeof(AdresseInternet));
    return 0;
}
