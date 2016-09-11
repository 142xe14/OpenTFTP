//
// Created by Julien Debroize on 11/09/2016.
//

#include "client.h"

SocketUDP sock;
AdresseInternet *dst = NULL;
char *ip, *filename, *destfile = NULL;
int *port = NULL;
size_t blksize, windowsize = (size_t) 0;

int main(int argc, char **argv) {
    // On récupère les arguments et on les envois à parse_args
    if (parse_args(argc, argv) != 0) {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }
    // On initialise la socket
    if (initSocket() != 0) {
        quit(EXIT_FAILURE);
    }
    // Gestion des signaux
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
    // On peut ensuite démarrer le transfert
    run();
    return EXIT_SUCCESS;
}

int parse_args(int argc, char **argv) {
    // On récupère l'adresse IP et le port
    ip = argv[1];
    if (ip == NULL) {
        return -1;
    }
    port = malloc(sizeof(int));
    *port = atoi(argv[2]);
    if (port == NULL) {
        return -1;
    }
    // On récupère les eventuelles options
    int ch;
    optind += 2; //On rajoute 2 car on a déjà IP et Port
    while ((ch = getopt(argc, argv, "b:w:")) != -1) {
        switch (ch) {
            case 'b':
                blksize = (size_t) atoi(optarg);
                break;
            case 'w':
                windowsize = (size_t) atoi(optarg);
                break;
            default:
                return -1;
        }
    }
    // On récupère le nom du fichier source et de destination
    filename = argv[optind];
    if (filename == NULL) {
        return -1;
    }
    destfile = argv[optind + 1];
    if (destfile == NULL) {
        return -1;
    }
    return 0;
}

int initSocket(void) {
    // On va créer la socket
    if (initSocketUDP(&sock) != 0) {
        fprintf(stderr, "initSocketUDP : impossible de créer la socket.\n");
        return -1;
    }
    dst = AdresseInternet_new(ip, (uint16_t) *port);
    if (dst == NULL) {
        fprintf(stderr, "AdresseInternet_new : impossible de créer une AdresseInternet.\n");
        return -1;
    }
    return 0;
}

void handle_sig(int sig) {
    if (sig == SIGINT || sig == SIGQUIT) {
        quit(EXIT_SUCCESS);
    }
}

void usage(char *progname) {
    fprintf(stderr, "Usage: %s ip port [-b blksize] [-w windowsize] src dest\n", progname);
}

void run(void) {
    AdresseInternet addrserv;
    ssize_t errcode;
    size_t buffer_len = (size_t) 512 + sizeof(uint16_t) * 2;
    char *buffer;
    // On construit le paquet RRQ avec éventuellement des options
    size_t rrqbuf_len = (size_t) 512;
    char rrqbuf[rrqbuf_len];
    if ((errcode = tftp_make_rrq_opt(rrqbuf, &rrqbuf_len, filename, blksize, windowsize)) != 0) {
        fprintf(stderr, "tftp_make_rrq_opt: %s\n", tftp_strerror(errcode));
        exit(EXIT_FAILURE);
    }
    // Si il y a des options, il faut envoyer un paquet RRQ et attendre un paquet OACK avant de recevoir le paquet DATA
    if (blksize != (size_t) 0 || windowsize != (size_t) 0) {
        buffer_len = (size_t) blksize + sizeof(uint16_t) * 2;
        buffer = malloc(sizeof(char) * buffer_len);
        printf("Paquet sent -->\n");
        tftp_print(rrqbuf);
        if ((errcode = tftp_send_RRQ_wait_OACK(&sock, dst, &addrserv, rrqbuf, rrqbuf_len, buffer, &buffer_len)) != 0) {
            fprintf(stderr, "tftp_send_RRQ_wait_OACK: %s\n", tftp_strerror(errcode));
            exit(EXIT_FAILURE);
        }
        printf("Paquet received -->\n");
        tftp_print(buffer);
        // On récupère les options dans le paquet OACK
        if ((errcode = tftp_get_opt(buffer, &blksize, &windowsize)) != 0) {
            fprintf(stderr, "tftp_get_opt: %s\n", tftp_strerror(errcode));
            exit(EXIT_FAILURE);
        }

        printf("%lu, %lu\n", blksize, windowsize);
        buffer_len = (size_t) blksize + sizeof(uint16_t) * 2;

        // On attend le premier paquet DATA
        if ((errcode = tftp_wait_DATA_with_timeout(&sock, &addrserv, buffer, &buffer_len)) != 0) {
            fprintf(stderr, "tftp_wait_DATA_with_timeout: %s\n", tftp_strerror(errcode));
            exit(EXIT_FAILURE);
        }
        printf("DATA length: %lu\n", buffer_len);
        printf("Paquet received -->\n");
        tftp_print(buffer);
    } else {
        blksize = (size_t) 512;
        windowsize = (size_t) 1;
        buffer = malloc(sizeof(char) * 512);
        printf("Paquet sent -->\n");
        tftp_print(rrqbuf);
        if ((errcode = tftp_send_RRQ_wait_DATA_with_timeout(&sock, dst, filename, &addrserv, buffer, &buffer_len)) != 0) {
            fprintf(stderr, "tftp_send_RRQ_wait_DATA_with_timeout: %s\n", tftp_strerror(errcode));
            quit(EXIT_FAILURE);
        }
        printf("Paquet received -->\n");
        tftp_print(buffer);
    }
    // On ouvre le fichier le destination
    int fd = open(destfile, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    //On controle que l'ouverture c'est bien passé
    if (fd == -1) {
        perror("open");
        quit(EXIT_FAILURE);
    }
    if (write(fd, buffer + sizeof(uint16_t) * 2, blksize) == -1) {
        perror("write");
        quit(EXIT_FAILURE);
    }
    uint16_t block = 1;
    while (1) {
        // On envoie le premier paquet ACK et attend le paquet DATA
        size_t acklen = (size_t) 512;
        char ackbuf[acklen];
        if ((errcode = tftp_make_ack(ackbuf, &acklen, block)) != 0) {
            fprintf(stderr, "tftp_make_ack : %s\n", tftp_strerror(errcode));
            quit(EXIT_FAILURE);
        }
        size_t data_len = blksize + sizeof(uint16_t) * 2;
        char data[data_len];
        printf("Paquet sent -->\n");

        tftp_print(ackbuf);
        if ((errcode = tftp_send_ACK_wait_DATA(&sock, &addrserv, ackbuf, acklen, data, &data_len)) != 0) {
            fprintf(stderr, "tftp_send_ACK_wait_DATA : %s\n", tftp_strerror(errcode));
            quit(EXIT_FAILURE);
        }
        printf("Paquet received -->\n");
        tftp_print(data);
        memcpy(&block, data + sizeof(uint16_t), sizeof(uint16_t));
        block = ntohs(block);
        if (write(fd, data + sizeof(uint16_t) * 2, data_len - sizeof(uint16_t) * 2) == -1) {
            perror("write");
            quit(EXIT_FAILURE);
        }
        if (data_len < blksize) {
            break;
        }
    }
    // On envoie le dernier paquet ACK
    buffer_len = 512;
    char ackbuf[buffer_len];
    if ((errcode = tftp_make_ack(ackbuf, &buffer_len, block)) != 0) {
        fprintf(stderr, "tftp_make_ack : %s\n", tftp_strerror(errcode));
        quit(EXIT_FAILURE);
    }
    printf("Paquet sent -->\n");
    tftp_print(ackbuf);
    if ((errcode = tftp_send_last_ACK(&sock, &addrserv, ackbuf, buffer_len)) != 0) {
        fprintf(stderr, "tftp_send_last_ACK : %s\n", tftp_strerror(errcode));
        quit(EXIT_FAILURE);
    }
    //On ferme le fichier
    if (close(fd) != 0) {
        perror("close");
        exit(EXIT_FAILURE);
    }
}

void quit(int code) {
    if (closeSocketUDP(&sock) != 0) {
        fprintf(stderr, "closeSocketUDP : impossible de fermer la socket.\n");
        exit(EXIT_FAILURE);
    }
    exit(code);
}