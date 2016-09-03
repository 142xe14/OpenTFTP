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

/**Ce programme sert de test à AdresseInternet.c
 *
 * @param argc
 * @param argv
 * @return 0 si le programme c'est correctement éxécuté
 */

int main(int argc, char* argv[]){

    printf("%d\n", argc);
    if(argc == 3)
        AdresseInternet_new(argv[0], argv[1]);
    else if(argc == 2) {
        AdresseInternet_any(argv[0]);
        AdresseInternet_loopback(argv[0]);
    }
    else{
        fprintf(stderr, "Erreur, mauvais nombre d'argument!\n");
        return -1;
    }

    return 0;

}