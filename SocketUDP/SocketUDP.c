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

int getLocalIP(const SocketUDP *sock, char *localIP, int tailleIP){
    char ip[16]; //IP = 15 caractères + echappement
    if(AdresseInternet_getIP(sock->addr,ip, sizeof(tailleIP)) == 0){
        strncpy(localIP, ip, tailleIP);
        return strlen(localIP);
    }
    else{
        return -1;
    }
}

uint16_t getLocalPort(const SocketUDP *sock){
    return AdresseInternet_getPort(sock->addr);
}

ssize_t writeToSocketUDP(SocketUDP *sock, const AdresseInternet *adresse, const char *buffer, int length){
    //On va controler que l'on a bien tous les paramètres de la fonction
    if (sock == NULL || adresse == NULL || buffer == NULL) {
        printf("Erreur, sock || adresse || buffer égal à null\n");
        return -1;
    }

    struct sockaddr_storage ss;
    if (AdresseInternet_to_sockaddr(adresse, (struct sockaddr *) &ss) != 0) {
        return -1;
    }
    socklen_t ss_len;
    //On controle pour savoir si l'adresse est IPV4 ou 6
    if (ss.ss_family == AF_INET6) {
        ss_len = sizeof(struct sockaddr_in6);
    } else {
        ss_len = sizeof(struct sockaddr);
    }

    ssize_t send = sendto(sock->sockfd, buffer, (size_t) length, 0, (struct sockaddr *) &ss,ss_len);

    //On controle la valeur de send pour vérifier que le sendto c'est bien passé
    if (send == -1){
        printf("Erreur dans sendto de write_socketuUDP! \n");
        return -1;
    }
    else{
        return send;
    }
}

ssize_t recvFromSocketUDP(SocketUDP *sock, char *buffer, int length, AdresseInternet *adresse, int timeout){
    //On vérifie d'abords que les arguments sont valides
    if (sock == NULL || buffer == NULL) {
        printf("Argument invalide pour recvFromSocketUDP, sock || buffer égal à NULL.\n");
        return -1;
    }

    struct sockaddr_storage ss;
    memset(&ss, 0, sizeof(ss));
    socklen_t ss_len = sizeof(ss);

    if (timeout > 0) {
        struct sigaction act;
        act.sa_handler = handleAlarm;
        act.sa_flags = 0;
        if (sigemptyset(&act.sa_mask) != 0) {
            printf("Erreur dans recvFromSocketUDP, sigemptyset n'est pas valide.\n");
            return -1;
        }
        if (sigaction(SIGALRM, &act, NULL) != 0) {
            printf("Erreur dans recvFromSocketUDP, sigaction n'est pas valide.\n");
            return -1;
        }
        alarm(timeout);
    }
    ssize_t receive = recvfrom(sock->sockfd, buffer, length, 0, (struct sockaddr *) &ss, &ss_len);
    if (timeout > 0) {
        alarm(0);
    }
    if (adresse != NULL) {
        sockaddr_to_AdresseInternet((struct sockaddr *) &ss, adresse);
    }
    return receive;
}

void handleAlarm(int sig){
    if (sig == SIGALRM) {
    }
}

int closeSocketUDP(SocketUDP *sock){
    if (close(sock->sockfd) != 0) {
        printf("Une erreur est survenue, la socket n'a pas pu être fermée correctement! \n");
        return -1;
    }
    else {
        return 0;
    }
}