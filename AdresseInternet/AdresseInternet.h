//
// Created by Julien Debroize on 02/09/2016.
//

#ifndef ADRESSEINTERNET_H
#define ADRESSEINTERNET_H

#include <stdint.h>

#include "AdresseInternetType.h"

/**La fonction ci-dessous construit une adresse internet à partir d'une éventuelle adresse(sous forme DNS ou IP) et d'un
 * numéro de port
 */

AdresseInternet * AdresseInternet_new(const char* adresse, uint16_t port);

/**La fonction AdresseInternet_any construit une adresse correspondant à toute les interfaces réseau à partir d'un
 * numéro de port
 */

AdresseInternet * AdresseInternet_any(uint16_t port);

/**La fonction AdresseInternet_loopback construit une adresse internet correspondant à l'interface loopback à partir
 * d'un numéro de port passé en paramètre
 */

AdresseInternet * AdresseInternet_loopback(uint16_t port);

/**La fonction AdresseInternet_getinfo extrait d'une adresse internet l'adresse réseau et transport correspondants*/

int AdresseInternet_getinfo(AdresseInternet *adresse, char *nomDNS, int tailleDNS, char *nomPort,
                                            int taillePort);

/**Extrait d'une adresse internet l'adrese IP correspondante sous forme de chaine de caractères*/
int AdresseInternet_getIP(const AdresseInternet *adresse, char *IP, int tailleIP);

/**Extrait d'une adresse le numéro de port*/
uint16_t AdresseInternet_getPort(const AdresseInternet *adresse);

/**Rend le domaine de communication de l'adresse passé en paramètre (AF_INET ou AF_INET6)*/
int AdresseInternet_getDomain(const AdresseInternet *adresse);

/**Construit une adresse internet à partir d'une structure sockaddr. La structure addr doit être suffisament grande
 * pour acceuillir l'adresse*/
int sockaddr_to_AdresseInternet(const struct sockaddr *addr, AdresseInternet *adresse);

/**Opposé fonction précédente*/
int AdresseInternet_to_sockaddr(const AdresseInternet *adresse, struct sockaddr *addr);

/**Compare deux adresse internet
 * Retourne 0 si différente
 * 1 si identique
 * -1 si erreur*/
int AdresseInternet_compare(const AdresseInternet *adresse1, const AdresseInternet *adresse2);

/**Cette fonctio n copie l'adresse internet adrsc dans la variable pointé par adrdst*/
int AdresseInternet_copy(AdresseInternet *adrdst, const AdresseInternet *adrsrc);
#endif //OPENTFTP_ADRESSEINTERNET_H
