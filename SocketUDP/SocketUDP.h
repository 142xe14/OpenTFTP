//
// Created by Julien Debroize on 04/09/2016.
//

#ifndef SOCKETUDP_H
#define SOCKETUDP_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "AdresseInternet.h"
#include "AdresseInternetType.h"
#include "SocketUDPType.h"

#define LOOPBACK 1

/**Fonction qui initialise un socket préalablement alloué*/
int initSocketUDP(SocketUDP *sock);

/**Fonction qui attache le socket sock à une adresse et port donnée. Si adresse et flag null alors elle est attaché
 * à toute les interfaces. si Flag = LOOPBACK alors elle est attaché à l'interface locale. Si adresse != null on ignore
 * flags
 */
int attacherSocketUDP(SocketUDP *sock, const char *adresse, uint16_t port, int flags);

/**Indique si la socket est attachée ou non, renvoie 0 si c'est le cas, -1 sinon*/
int estAttachee(SocketUDP *sock);

/**remplie buffer préalloué de longueur taille avec le nom de la Socket UDP. Si le buffer est trop petit, le nom est
 * tronqué, si besoin, socket est mis à jour.*/
int getLocalName(SocketUDP *sock, char *buffer, int taille);
#endif //OPENTFTP_SOCKETUDP_H
