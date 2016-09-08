//
// Created by Julien Debroize on 02/09/2016.
//

#ifndef OPENTFTP_ADRESSEINTERNETTYPE_H
#define OPENTFTP_ADRESSEINTERNETTYPE_H

#define _DNS_NAME_MAX_SIZE 256
#define _SERVICE_NAME_MAX_SIZE 20

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>


typedef struct {
    struct sockaddr_storage sockAddr;
    char nom[_DNS_NAME_MAX_SIZE];
    char service[_SERVICE_NAME_MAX_SIZE];
} _adresseInternet_struct;

typedef _adresseInternet_struct AdresseInternet;
#endif //OPENTFTP_ADRESSEINTERNETTYPE_H
