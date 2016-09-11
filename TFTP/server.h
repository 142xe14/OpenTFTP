//
// Created by Julien Debroize on 11/09/2016.
//

#ifndef OPENTFTP_SERVER_H
#define OPENTFTP_SERVER_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "tftp.h"

#define SOCK_BIND 0
#define SOCK_NO_BIND 1

//Structure contenant les infos de la requetes
struct t_rrq {
    SocketUDP *sock;
    AdresseInternet *addr;
    char *rrqbuf;
    size_t rrqlen;
    char *filename;
    size_t blksize;
    size_t windowsize;
};

/** Fonction qui crée et attache une socket*/
int initSocket(SocketUDP *sock, int bind);

/**Permet de quitter le programme en libérant la socket**/
void quit(int code);

/**Boucle principale du programme*/
void *process_RRQ(void *arg);

/**Permet d'attendre une requete RRQ*/
void handle_RRQ(void);

/**Permet de ferme le programme via des signaux*/
void handle_sig(int sig);

#endif //OPENTFTP_SERVER_H
