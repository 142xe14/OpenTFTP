//
// Created by Julien Debroize on 11/09/2016.
//

#ifndef OPENTFTP_CLIENT_H
#define OPENTFTP_CLIENT_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "tftp.h"

/**Fonction qui récupère les arguments et vérifie qu'ils sont correctes*/
int parse_args(int argc, char **argv);

/**Affiche l'usage du programme*/
void usage(char *progname);

/**Initialise la socket*/
int initSocket(void);

/**Quitte le programme*/
void quit(int code);

/**Permet de démarrer le transfert*/
void run(void);

/**Quitte le programme à la réception d'un signal*/
void handle_sig(int sig);

#endif //OPENTFTP_CLIENT_H
