//
// Created by Julien Debroize on 04/09/2016.
//

#include "SocketUDP.h"

int initSocketUDP(SocketUDP *sock){
    sock->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    //Si sockfd de socket = -1, une erreur est survenue lors de l'initialisation
    if(sock->sockfd == -1){
        printf("Erreur dans initSocketUDP \n");
        return -1;
    }

    //on met bound à -1 pour nous rappeler que la socket n'est pas attachée
    sock->bound = -1;
    return 0;
}

int attacherSocketUDP(SocketUDP *sock, const char *adresse, uint16_t port, int flags){
    //On regarde d'abords si une adresse est présente
    if(adresse != NULL){
        sock->addr = AdresseInternet_new(adresse, port);
        //On véririe que tout c'est bien passé
        if(sock->addr == NULL){
            printf("Erreur dans attacher socket(new), sock->addr = NULL! \n");
            return -1;
        }
    }
    else if(flags == 0){
        //Si flags est nul, il faut crée attacher la socket à toutes les interfaces
        sock->addr = AdresseInternet_any(port);
        if(sock->addr == NULL){
            printf("Erreur dans attacher socket(any), sock->addr = NULL! \n");
            return -1;
        }
    }
    else if(flags == LOOPBACK){
        sock->addr = AdresseInternet_loopback(port);
        if(sock->addr == NULL){
            printf("Erreur dans attacher socket(loopback), sock->addr = NULL! \n");
            return -1;
        }
    }

    struct sockaddr_storage ss;
    if (AdresseInternet_to_sockaddr(sock->addr, (struct sockaddr *) &ss) != 0) {
        return -1;
    }

    //On controle la taille (ipv4 ou 6) pour le bind à venir
    socklen_t ss_len;
    if (ss.ss_family == AF_INET6) {
        ss_len = sizeof(struct sockaddr_in6);
    } else {
        ss_len = sizeof(struct sockaddr);
    }

    if (bind(sock->sockfd, (struct sockaddr *) &ss, ss_len) != 0) {
        printf("Erreur dans attacher socket, echec du bind! \n");
        return -1;
    }

    //On met bound à 0 pour indiquer que notre socket est bien attachée
    sock->bound = 0;
    return 0;
}

int estAttachee(SocketUDP *sock){
    if(sock->bound == 0)
        return 0;
    else
        return -1;
}

int getLocalName(SocketUDP *sock, char *buffer, int taille){
    char nom[taille];
    char ip[1];
    if (AdresseInternet_getinfo(sock->addr, nom, taille, ip, 0) != 0) {
        return -1;
    }
    strncpy(buffer, nom, (size_t) taille);
    return strlen(buffer);
}
