//
// Created by Julien Debroize on 02/09/2016.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

#include "AdresseInternet.h"

/** RAPPEL PROVENANT DU MAN
   int getaddrinfo(const char *node, const char *service,
                const struct addrinfo *hints,
                struct addrinfo **res);

   Étant donné node et service, qui identifie un hôte Internet et un service, getaddrinfo() renvoie une ou
   plusieurs structures addrinfo, chacune d'entre elles contenant une adresse Internet qui puisse être indiquée
   dans un appel à bind(2) ou connect(2). La fonction getaddrinfo() combine les possibilités offertes par les
   fonctions getservbyname(3) et getservbyport(3) en une unique interface, mais contrairement à ces dernières fonctions,
   getaddrinfo() est réentrante et permet aux programmes d'éliminer les dépendances IPv4/IPv6.
 */

AdresseInternet * _AdresseInternetNew(int flag, const char* nom, uint16_t port){
    struct addrinfo hints;
    //memset remplit sizeof(struct addrinfo) octets de la zone mémoire pointé par hints
    memset(&hints, 0, sizeof(struct addrinfo));

    char numport[4];
    snprintf(numport,4,"%d", port);
    struct addrinfo *result;
    int r = getaddrinfo(nom, numport, &hints, &result);
    //Si r est différent de 0, une erreur est survenue das getaddrinfo
    if(r != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(r));
        return NULL;
    }

    //Il peut arriver que result ne soit ni une adresse ipv4 ni ipv6, on va alors indiquer une erreur
    if(result->ai_family != AF_INET && result->ai_family != AF_INET6){
        fprintf(stderr, "adresse invalide!\n");
        return NULL;
    }

    AdresseInternet * adresse = malloc(sizeof(AdresseInternet));
    adresse->nom[0] = 0;
    adresse->service[0] = 0;
    memcpy(&(adresse->sockAddr), result->ai_addr, result->ai_addrlen);
    freeaddrinfo(result);
    printf("Adresse correctement crée et libéré");
    return adresse;
}

AdresseInternet * AdresseInternet_new(const char* nom, uint16_t port){
    return _AdresseInternetNew(AI_PASSIVE, nom, port);
}

AdresseInternet * AdresseInternet_any(uint16_t port){
    return _AdresseInternetNew(AI_PASSIVE, NULL, port);
}

AdresseInternet * AdresseInternet_loopback(uint16_t port){
    return _AdresseInternetNew(0, NULL, port);
}

/** AdresseInternet_free libère la mémoire allouée par les fonctions précédentes*/

void AdresseInternet_free(AdresseInternet *adresse){
    free(adresse);
}

int AdresseInternet_getinfo(AdresseInternet *adresse, char *nomDNS, int tailleDNS, char *nomPort,
                            int taillePort){
    char *adresseNomDNS;
    char *adresseNomPort;
    //Dans un premier temps, on regarde si le nom DNS n'est pas déjà présent dans la structure
    if (adresse->nom[0] != 0){
        //On copie le nom de la structure dans nomDNS
        strncpy(nomDNS, adresse->nom, tailleDNS);
        //L'énoncé demande à ce que nomDNS ce termine par un 0
        //Comme strncpy a copié adresse->nom avec le caractère d'échappement à la fin, on remplace le caractère à
        //tailleDNS - 1 par 0
        nomDNS[tailleDNS - 1] = 0;
        adresseNomDNS = NULL;
    }
    else{
        adresseNomDNS = nomDNS;
    }
    //On regarde ensuite si le nom du Port est présent dans la structure
    if(adresse->service[0] != 0){
        //On copie le nom du port dans nomPort
        strncpy(nomPort, adresse->service, taillePort);
        //L'énoncé demande à ce que nomPort ce termine par un 0
        //Comme strncpy a copié adresse->service avec le caractère d'échappement à la fin, on remplace le caractère à
        //taillePort - 1 par 0
        nomPort[taillePort - 1] = 0;
        adresseNomPort = NULL;
    }
    else{
        adresseNomPort = nomPort;
    }
    //Si la condition ci-dessous est vrai cela indique que :
    //La structure adresse n'avais pas de nom et de service
    //Le nom et le service n'ont pas été passé en paramètre
    if(adresseNomDNS == NULL && adresseNomPort == NULL){
        printf("Une erreur est survenue dans AdresseInternet_getinfo()\n");
        return -1;
    }

    //getnameinfo, fonction opposé de getaddrinfo
    if(getnameinfo((struct sockaddr *)&adresse->sockAddr, sizeof(adresse->sockAddr), adresseNomDNS, tailleDNS,
                            adresseNomPort, taillePort, 0) != 0){
        fprintf(stderr, "Erreur survenue dans getnameinfo\n");
        return - 1;
    }
    //On va rentrer les valeurs trouver par getnameinfo dans notre structure adresse
    strncpy(adresse->nom, adresseNomDNS, tailleDNS);
    strncpy(adresseNomPort, adresseNomPort, taillePort);
    printf("getinfo c'est bien déroulé! \n");
    return 0;
}
