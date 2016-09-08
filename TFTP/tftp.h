//
// Created by Julien Debroize on 05/09/2016.
//

#ifndef OPENTFTP_TFTP_H
#define OPENTFTP_TFTP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "SocketUDP.h"
#include "AdresseInternet.h"

//Liste entêtes OPcode
#define RRQ 1
#define WRQ 2
#define DATA 3
#define ACK 4
#define ERROR 5
#define OACK 6

//Liste erreurs liée a TFTP
#define UNDEF 0
#define FILNF 1
#define ILLEG 4
#define UNKNOW 5
#define ERROACK 8

//Liste erreurs liée aux utlisations
#define ERRARG 1
#define ERRCO 2
#define ERRSEND 3
#define ERRRECEIVE 4
#define ERRTIMO 5
#define ERRINVA 6
#define ERRUNKN 7
#define ERRNOPA 8

//Config pour le serveur
//Temps avant Timeout
#define TIMEOUT 5
#define MAX_TRIES 3 //NB_Essai Max
#define MIN_BLKSIZE 8 //Taille block mini
#define MAX_BLKSIZE 65464 //Taille block maxi
#define MIN_WINDOWSIZE 1 //Mini windowsize
#define MAX_WINDOWSIZE 65535 //maxi windowsize

//On fait une variable global pour contenir les éventuelles erreurs
extern char *errors[];

/**Fonction permettant l'envoi d'un packet d'acquittement*/
int tftp_make_ack(char *buffer, size_t *length, uint16_t block);

/**Fonction permettant l'envoie d'une requete*/
int tftp_make_rrq(char *buffer, size_t *length, char *fichier);

/**Fonction pour l'envoie de packet Data*/
int tftp_make_data(char *buffer, size_t *length, uint16_t block, const char *data, size_t n);

/**Fonction pour l'envoie de packet d'erreur*/
int tftp_make_error(char *buffer, size_t *length, uint16_t errorcode, const char *message);

/**fonction qui envoie un message d'erreur à l'adresse dst*/
void tftp_send_error(SocketUDP *sock, const AdresseInternet *dst, uint16_t code, const char *msg);

/**Envoie un demande de connexion RRQ pour lire le fichier sur la socket sock à l'adresse dst. La fonction place
 * en attente de réponse au plus TIMEOUT secondes. Si elle reçoit une réponse valide (de type DATA 1), l'adresse de
 * l'expediteur est renseignée dans la variable connexion, le packet reçu dans réponse et sa taille dans replength.
 * Si elle reçoit une réponse non valide, un packet d'erreur tftp est envoyé à l'expediteur et les variables
 * connexion, reponse, replength sont indeterminees.*/
int tftp_send_RRQ_wait_DATA_with_timeout(SocketUDP *sock, const AdresseInternet *dst, const char *fichier,
                                        AdresseInternet *connexion, char *reponse, size_t *replength);

/**Idem fonction précédente mais renvoie une demande de connexion en cas de dépassement du temps*/
int tftp_send_RRQ_wait_DATA(SocketUDP *sock, const AdresseInternet *dst, const char *fichier,
                                         AdresseInternet *connexion, char *reponse, size_t *replength);

#endif //OPENTFTP_TFTP_H
