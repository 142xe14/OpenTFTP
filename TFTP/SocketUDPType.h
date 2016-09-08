//
// Created by Julien Debroize on 04/09/2016.
//

#ifndef OPENTFTP_SOCKETUDPTYPE_H_H
#define OPENTFTP_SOCKETUDPTYPE_H_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "AdresseInternet.h"
#include "AdresseInternetType.h"

typedef struct SockerUDP{
    int sockfd;
    AdresseInternet *addr;
    int bound;
} SocketUDP;
#endif //OPENTFTP_SOCKETUDPTYPE_H_H
