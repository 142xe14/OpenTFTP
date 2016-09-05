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

/**remplie localIP avec l'IP locale de la socketUDP*/
int getLocalIP(const SocketUDP *sock, char *localIP, int tailleIP);

/**Retourne le port de la socket*/
uint16_t getLocalPort(const SocketUDP *sock);

/**Ecrit sur la socket sock vers adresse un bloc d'octet buffer de taille length et retourne la taille des données
 * écrite
 * @param sock
 * @param adresse
 * @param buffer
 * @param length
 * @return ssize_t
 */
ssize_t writeToSocketUDP(SocketUDP *sock, const AdresseInternet *adresse, const char *buffer, int length);

/**Lit sur sock les données aenvoyées par une machine d'adresse adresse, fonction bloquante pendant timeout secondes.
 * Elle lit et place dans buffer un bloc d'octets de taille au plus length. La fonction recvFromSocketUDP retourne la
 * taille des données lues ou -1 s'il y a erreur.
 * @param socket
 * @param buffer
 * @param length
 * @param adresse
 * @param timeout
 * @return ssize_t
 */
ssize_t recvFromSocketUDP(SocketUDP *sock, char *buffer, int length, AdresseInternet *adresse, int timeout);

void handleAlarm(int sig);

int closeSocketUDP(SocketUDP *sock);

#endif //OPENTFTP_SOCKETUDP_H
