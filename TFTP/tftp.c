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
    *length += sizeof(uint16_t);
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
    if (buffer == NULL || length == NULL || *length > 512 || message == NULL) {
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
    errorcode = htons(errorcode);
    memcpy(buffer + *length, &errorcode, sizeof(uint16_t));
    *length += sizeof(uint16_t);
    memcpy(buffer + *length, message, strlen(message) +1);
    *length += strlen(message) + 1;

    return 0;
}

int tftp_send_error(SocketUDP *sock, const AdresseInternet *dst, uint16_t code, const char *msg){
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
    char res_buf[*replength];
    size_t count;
    if ((count = recvFromSocketUDP(sock, res_buf, *replength, &con_buf, TIMEOUT)) == (size_t) -1){
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
    *replength = count;

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

int tftp_send_DATA_wait_ACK(SocketUDP *sock, const AdresseInternet *dst, const char *paquet, size_t paquetlen){
    // On vérifie que les arguments passé sont correctes
    if(sock == NULL || dst == NULL || paquet == NULL){
        return ERRARG;
    }
    // On vérifie que le paquet donné et de type DATA
    uint16_t opcode;
    memcpy(&opcode, paquet, sizeof(uint16_t));
    if (opcode != htons(DATA)) {
        return ERRINVA;
    }
    // Si c'est le cas, on envoie le paquet DATA
    size_t tries = 0;
    if (writeToSocketUDP(sock, dst, paquet, paquetlen) < 0) {
        return ERRSEND;
    }
    while (tries <= MAX_TRIES) {
        // On attend le paquet retour
        size_t length = (size_t) 512;
        char buffer[length];
        if (recvFromSocketUDP(sock, buffer, length, NULL, TIMEOUT) < 0) {
            tries++;
            continue;
        }
        // On vérifie qu'il s'agit bien d'un paquet ACK
        memcpy(&opcode, buffer, sizeof(uint16_t));
        opcode = ntohs(opcode);
        if (opcode != ACK) {
            tftp_send_error(sock, dst, ILLEG, "Le pacquet reçu n'est pas ACK!.");
            tries++;
            continue;
        } else {
            // On vérifie enfin que les numéros de bloc sont correctes
            uint16_t blockDATA;
            memcpy(&blockDATA, paquet + sizeof(uint16_t), sizeof(uint16_t));
            blockDATA = ntohs(blockDATA);
            uint16_t blockACK;
            memcpy(&blockACK, buffer + sizeof(uint16_t), sizeof(uint16_t));
            blockACK = ntohs(blockACK);
            if (blockDATA == blockACK) {
                return 0;
            }
        }
    }
    return ERRNOPA;
}

int tftp_send_ACK_wait_DATA(SocketUDP *sock, const AdresseInternet *dst, const char *paquet, size_t paquetlen,
                            char *res, size_t *reslen) {
    // On vérifie que les arguments passé sont correctes
    if (sock == NULL || dst == NULL || paquet == NULL) {
        return ERRARG;
    }
    // On vérifie qu'il s'agit bien d'un paquet ACK
    uint16_t opcode;
    memcpy(&opcode, paquet, sizeof(uint16_t));
    if (opcode != htons(ACK)) {
        return ERRINVA;
    }
    // On envoie le paquet ACK
    if (writeToSocketUDP(sock, dst, paquet, paquetlen) == -1) {
        return ERRSEND;
    }
    // On attend ensuite le paquet DATA
    size_t tries = 0;
    while (tries < MAX_TRIES) {
        size_t length = *reslen;
        char buffer[length];
        AdresseInternet from;
        if ((length = recvFromSocketUDP(sock, buffer, length, &from, TIMEOUT)) == (size_t) -1) {
            tries++;
            continue;
        }
        memcpy(res, buffer, length);
        *reslen = (size_t) length;
        return 0;
    }
    return ERRTIMO;
}

int tftp_send_last_ACK(SocketUDP *sock, const AdresseInternet *dst, const char *paquet, size_t paquetlen) {
    // On vérifie que les arguments passé sont correctes
    if (sock == NULL || dst == NULL || paquet == NULL) {
        return ERRARG;
    }
    // On vérifie qu'il s'agit bien d'un paquet ACK
    uint16_t opcode;
    memcpy(&opcode, paquet, sizeof(uint16_t));
    if (opcode != htons(ACK)) {
        return ERRINVA;
    }
    // On envoie le paquet ACK
    if (writeToSocketUDP(sock, dst, paquet, paquetlen) == -1) {
        return ERRSEND;
    }
    return 0;
}