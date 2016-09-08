//
// Created by Julien Debroize on 05/09/2016.
//

#include "tftp.h"

char *errors[] = {
        NULL,
        "Argument invalide",
        "Code d'erreur invalide",
        "Impossible d'envoyer le paquet",
        "Impossible de recevoir le paquet",
        "Le délai d'attente est dépassé",
        "Le paquet est invalide",
        "Erreur inconnue",
        "Aucun paquet reçu",
};


int tftp_make_ack(char *buffer, size_t *length, uint16_t block) {
    // On vérifie que les arguments passé sont correctes
    if (buffer == NULL || length == NULL || *length > 512 || block < 1) {
        return ERRARG;
    }

    //Si c'est le cas, on peut créer notre paquet
    //On remplit les length premier octets de la zone mémoire du buffer avec des 0
    memset(buffer, 0, *length);
    *length = 0;
    uint16_t opcode = htons(ACK);
    //On copie l'opcode dans buffer
    memcpy(buffer, &opcode, sizeof(uint16_t));
    *length += sizeof(uint_16_t);
    //La fonction htons()  convertit  un  entier  court  (short)
    //depuis  l'ordre des octets de l'hôte vers celui du réseau.
    block = htons(block);
    memcpy(buffer + *length, &block, sizeof(uint16_t));
    *length += sizeof(uint16_t);

    return 0;
}

int tftp_make_rrq(char *buffer, size_t *length, const char *fichier) {
    // On vérifie que les arguments passé sont correctes
    if (buffer == NULL || length == NULL || *length > 512 || fichier == NULL) {
        return ERRARG;
    }

    //Si c'est le cas, on peut créer notre paquet
    memset(buffer, 0, *length);
    *length = 0;
    uint16_t opcode = htons(RRQ);
    memcpy(buffer, &opcode, sizeof(uint16_t));
    *length += sizeof(uint16_t);
    memcpy(buffer + *length, fichier, strlen(fichier) + 1);
    *length += sizeof(char) * (strlen(fichier) + 1);
    memcpy(buffer + *length, "octet", strlen("octet") + 1);
    *length += sizeof(char) * (strlen("octet") + 1);

    return 0;
}

int tftp_make_data(char *buffer, size_t *length, uint16_t block, const char *data, size_t n){
    // On vérifie que les arguments passé sont correctes
    if (buffer == NULL || length == NULL || *length > 512 || data == NULL) {
        return ERRARG;
    }

    //Si c'est le cas, on peut créer notre paquet
    memset(buffer, 0, *length);
    *length = 0;
    uint16_t opcode = htons(DATA);
    memcpy(buffer, &opcode, sizeof(uint16_t));
    *length += sizeof(uint16_t);
    block = htons(block);
    memcpy(buffer + *length, &block, sizeof(uint16_t));
    *length += sizeof(uint16_t);
    memcpy(buffer + *length, data, n);
    *length += n;

    return 0;
}

int tftp_make_error(char *buffer, size_t *length, uint16_t errorcode, const char *message){
    // On vérifie que les arguments passé sont correctes
    if (buffer == NULL || length == NULL || *length > 512 || data == NULL) {
        return ERRARG;
    }

    //On vérifie qu le code d'eereur est connu
    if (errorcode != UNDEF && errorcode !=FILNF && errorcode != ILLEG && errorcode != UNKNOW){
        return ERRCO;
    }

    //Si c'est le cas, on peut créer notre paquet
    memset(buffer, 0, *length);
    *length = 0;
    uint16_t opcode = htons(ERROR);
    memcpy(buffer, &opcode, sizeof(uint16_t));
    *length += sizeof(uint16_t);
    block = htons(errorcode);
    memcpy(buffer + *length, &errorcode, sizeof(uint16_t));
    *length += sizeof(uint16_t);
    memcpy(buffer + *length, message, strlen(message) +1);
    *length += strlen(message) + 1;

    return 0;
}

void tftp_send_error(SocketUDP *sock, const AdresseInternet *dst, uint16_t code, const char *msg){
    // On vérifie que les arguments passé sont correctes
    if (sock == NULL || dst == NULL || msg == NULL) {
        return ERRARG;
    }

    //On va construire notre packet
    size_t length = 512;
    char buffer[length];
    size_t errcode;
    if ((errcode = tftp_make_error(buffer, &length, code, msg)) != 0){
        return errcode;
    }

    //On envoie le packet
    if (writeToSocketUDP(sock, dst, buffer, length) == -1){
        return ERRSEND;
    }

    return 0;
}

int tftp_send_RRQ_wait_DATA_with_timeout(SocketUDP *sock, const AdresseInternet *dst, const char *fichier,
                                         AdresseInternet *connexion, char *reponse, size_t *replength){
    // On vérifie que les arguments passé sont correctes
    if(sock == NULL || dst == NULL || fichier == NULL || connexion == NULL || reponse == NULL || replength == NULL){
        return ERRARG;
    }

    //On construit notre packet RRQ
    size_t length = 512;
    char buffer[length];
    size_t errcode;
    if((errcode = tftp_make_rrq(buffer, &length, fichier)) != 0){
        return errcode;
    }

    //On envoie notre packet RRQ
    if(writeToSocketUDP(sock, dst, buffer, length) == -1){
        return ERRSEND;
    }

    //On attend la réponse
    AdresseInternet con_buf;
    char res_buf[*reslen];
    size_t count;
    if ((count = recvFromSocketUDP(sock, res_buf, *reslen, &con_buf, TIMEOUT)) == (size_t) -1){
        if(errno == EINTR){
            return ERRTIMO;
        }
        else{
            return ERRRECEIVE;
        }
    }

    if (AdresseInternet_copy(connexion, &con_buf) != 0){
        return ERRUNKN;
    }
    memcpy(reponse, res_buf, count);
    *reslen = count;

    return 0;
}

int tftp_send_RRQ_wait_DATA(SocketUDP *sock, const AdresseInternet *dst, const char *fichier,
                            AdresseInternet *connexion, char *reponse, size_t *replength){
    // On vérifie que les arguments passé sont correctes
    if(sock == NULL || dst == NULL || fichier == NULL || connexion == NULL || reponse == NULL || replength == NULL){
        return ERRARG;
    }
    // On envoie le packet RRQ. Si une erreur survient, on recommence.
    //On utilise un do -> while, cela permet d'envoyer une première requete quoi qu'il arrive
    unsigned int tries = 0;
    size_t errorcode;
    do {
        if ((errorcode = tftp_send_RRQ_wait_DATA_with_timeout(sock, dst, fichier, connexion, reponse, replength)) == 0) {
            return 0;
        }
        tries++;
    } while (tries < MAX_TRIES);
    return errorcode;

}
}