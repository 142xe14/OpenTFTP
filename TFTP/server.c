//
// Created by Julien Debroize on 11/09/2016.
//

#include "server.h"

int port;
SocketUDP *sock = NULL;

int main(int argc, char **argv) {
    // On vérifie que les arguments sont correctes
    if (argc != 2) {
        fprintf(stderr, "%s : port\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    // On récupère les arguments, notamment le numéro de port
    port = atoi(argv[1]);
    // On peut ensuite initialiser notre socket
    sock = malloc(sizeof(SocketUDP));
    if (initSocket(sock, SOCK_BIND) != 0) {
        quit(EXIT_FAILURE);
    }

    // On s'occupe ensuite de la mise en place des signaux
    struct sigaction sa;
    sa.sa_handler = handle_sig;
    if (sigfillset(&sa.sa_mask) != 0) {
        perror("sigfillset");
        quit(EXIT_FAILURE);
    }
    if (sigaction(SIGINT, &sa, NULL) != 0) {
        perror("sigaction");
        quit(EXIT_FAILURE);
    }
    if (sigaction(SIGQUIT, &sa, NULL) != 0) {
        perror("sigaction");
        quit(EXIT_FAILURE);
    }

    // On attend ensuite une requete RRQ
    while (1) {
        handle_RRQ();
    }
    quit(EXIT_SUCCESS);
}

void handle_sig(int sig) {
    if (sig == SIGINT || sig == SIGQUIT) {
        quit(EXIT_SUCCESS);
    }
}

int initSocket(SocketUDP *sock, int bind) {
    // On va créer notre socket
    if (initSocketUDP(sock) != 0) {
        fprintf(stderr, "initSocketUDP : impossible de créer la socket.\n");
        return -1;
    }
    // Une fois la socket créer, on va l'attacher
    if (bind == SOCK_BIND) {
        if (attacherSocketUDP(sock, NULL, port, 0) != 0) {
            fprintf(stderr, "attacherSocketUDP : impossible d'attacher la socket.\n");
            return -1;
        }
    }
    return 0;
}

void handle_RRQ(void) {

    // On attend une requête RRQ
    size_t rrq_len = 512;
    char *rrq_buf = malloc(sizeof(char) * rrq_len);
    AdresseInternet *addr_cli = malloc(sizeof(AdresseInternet));
    if (recvFromSocketUDP(sock, rrq_buf, rrq_len, addr_cli, -1) == -1) {
        fprintf(stderr, "recvFromSocketUDP : impossible de recevoir le paquet.\n");
        return;
    }
    printf("Received packet -->\n");
    tftp_print(rrq_buf);
    // On va vérifier que l'on a bien reçu un paquet RRQ
    uint16_t opcode;
    memcpy(&opcode, rrq_buf, sizeof(uint16_t));
    opcode = ntohs(opcode);
    //Si notre paquet n'est pas de type RRQ, on retourne une erreur
    if (opcode != RRQ) {
        tftp_send_error(sock, addr_cli, ILLEG, "Le serveur attend un paquet RRQ.");
        return;
    }
    // On va lancer le traitement du paquet dans un nouveau thread. Cela permet de traiter plusieurs paquet
    // simultanément
    struct t_rrq *rrq = malloc(sizeof(struct t_rrq));
    rrq->blksize = (size_t) 0;
    rrq->windowsize = (size_t) 0;
    rrq->sock = sock;
    rrq->addr = addr_cli;
    rrq->rrqbuf = rrq_buf;
    rrq->rrqlen = rrq_len;
    rrq->filename = malloc(sizeof(char) * strlen(rrq_buf + sizeof(uint16_t)) + 1);
    strncpy(rrq->filename, rrq_buf + sizeof(uint16_t), strlen(rrq_buf + sizeof(uint16_t)) + 1);
    pthread_t thread;
    //On créer notre nouveau thread, qui appelle la fonction process_RRQ
    if (pthread_create(&thread, NULL, process_RRQ, rrq) != 0) {
        perror("pthread_create");
        return;
    }
}

void *process_RRQ(void *arg) {
    struct t_rrq *rrq = (struct t_rrq *) arg;
    SocketUDP *sock = rrq->sock;
    SocketUDP *sock_cli;
    AdresseInternet *addr_cli = rrq->addr;
    char *filename = rrq->filename;
    size_t blksize = rrq->blksize;
    size_t windowsize = rrq->windowsize;
    // On va récuperer les options de la requête
    size_t errcode;
    if ((errcode = tftp_get_opt(rrq->rrqbuf, &blksize, &windowsize)) != 0) {
        fprintf(stderr, "tftp_get_opt: %s\n", tftp_strerror(errcode));
        return NULL;
    }
    // On va initialiser la nouvelle socket dédiée au client
    sock_cli = malloc(sizeof(SocketUDP));
    if (initSocket(sock_cli, SOCK_NO_BIND) != 0) {
        tftp_send_error(sock, addr_cli, UNDEF, "Une erreur de connexion est survenue.");
        return NULL;
    }
    // Si il y avait des options, on construit et envoie un pacquet OACK
    if (blksize != (size_t) 0 || windowsize != (size_t) 0) {
        size_t oackbuf_len = (size_t) 512;
        char oackbuf[oackbuf_len];
        size_t errcode;
        if ((errcode = tftp_make_oack(oackbuf, &oackbuf_len, blksize, windowsize)) != 0) {
            fprintf(stderr, "tftp_make_oack: %s\n", tftp_strerror(errcode));
            return NULL;
        }
        printf("Paquet sent -->\n");
        tftp_print(oackbuf);
        if ((errcode = tftp_send_oack(sock_cli, addr_cli, oackbuf, oackbuf_len)) != 0) {
            fprintf(stderr, "tftp_send_oack: %s\n", tftp_strerror(errcode));
            return NULL;
        }
    }
    else {
        blksize = (size_t) 512;
        windowsize = (size_t) 1;
    }
    // On va ouvrir le fichier qui a été demandé
    int fd = open(filename, O_RDONLY);
    //On controle que l'ouverture à réussi
    if (fd == -1) {
        perror("fd");
        if (errno == EACCES) {
            tftp_send_error(sock, addr_cli, FILNF, "Le fichier demandé est introuvable.");
        }
        return NULL;
    }
    if (fd != -1) {
        //Si l'on arrive ici, le fichier à été ouvert. On va donc le lire
        char fcontent_buf[blksize];
        size_t count;
        uint16_t block = 1;
        while ((count = read(fd, fcontent_buf, sizeof(fcontent_buf))) > 0) {
            // On construit un paquet DATA
            size_t data_len = blksize + sizeof(uint16_t) * 2;
            char data_buf[data_len];
            ssize_t errcode;
            if ((errcode = tftp_make_data(data_buf, &data_len, block, fcontent_buf, count)) != 0){
                fprintf(stderr, "tftp_make_data : %s\n", tftp_strerror(errcode));
                tftp_send_error(sock_cli, addr_cli, UNDEF, "Une erreur est survenue.");
                break;
            }
            printf("Paquet sent -->\n");
            tftp_print(data_buf);
            // On envoie le paquet DATA et l'on va attendre le paquet ACK
            if ((errcode = tftp_send_DATA_wait_ACK(sock_cli, addr_cli, data_buf, data_len)) != 0) {
                fprintf(stderr, "tftp_send_DATA_wait_ACK : %s\n", tftp_strerror(errcode));
                tftp_send_error(sock_cli, addr_cli, UNDEF, "Une erreur est survenue.");
                break;
            }
            printf("Paquet reçu -->\n");
            printf("ACK - %d\n", block);
            block++;
        }
        // Ensuite, on va fermer la socket dédiée au client
        if (closeSocketUDP(sock_cli) != 0) {
            fprintf(stderr, "closeSocketUDP : impossible de fermer la socket.\n");
        }
        sock_cli = NULL;
        // On ferme enfin le fichier
        if (close(fd) != 0) {
            perror("close");
        }
    }
    return NULL;
}

void quit(int code) {
    if (sock != NULL) {
        if (closeSocketUDP(sock) == -1) {
            fprintf(stderr, "closeSocketUDP : impossible de fermer la socket du serveur.\n");
            exit(EXIT_FAILURE);
        }
        free(sock);
    }
    exit(code);
}