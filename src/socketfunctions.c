#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

#ifndef SOCKET_ERROR
#include "sockethead.h"
#endif

int time_out(SOCKET sock) {
    fd_set rfds;
    struct timeval tv;
    int retval;

    /* Surveiller stdin (fd 0) en attente d'entrées */
    FD_ZERO(&rfds);
    FD_SET(sock, &rfds);

    /* Pendant SOCKET_TIME_OUT secondes maxi */
    tv.tv_sec = SOCKET_TIME_OUT;
    tv.tv_usec = 0;
    retval = select(sock + 1, &rfds, NULL, NULL, &tv);

    return retval;
}

/*********** SOCKET UDP *************/

SOCKET initSocketUDP(void) {
    return socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

SOCKET bindSocketUDP(SOCKET sock, int port) {
    SOCKADDR_IN p_adresse;
    bzero(&p_adresse, sizeof (SOCKADDR_IN));
    p_adresse.sin_addr.s_addr = INADDR_ANY;
    p_adresse.sin_family = AF_INET;
    p_adresse.sin_port = htons(port);

    return bind(sock, (SOCKADDR*) & p_adresse, sizeof (SOCKADDR_IN));
}

SOCKET sendToUDP(SOCKET sock, OBJET * msg, char *host, int port) {
    int lg;
    SOCKADDR_IN p_adresse;
    HOSTENT * h = gethostbyname(host);
    bcopy(h->h_addr, &p_adresse.sin_addr, h->h_length);
    p_adresse.sin_family = AF_INET;
    p_adresse.sin_port = htons(port);

    lg = sizeof (SOCKADDR_IN);
    return sendto(sock, msg, sizeof (OBJET), 0, (SOCKADDR*) & p_adresse, lg);
}

SOCKET recvFromPUDP(SOCKET sock, OBJET * msg, char *host, int port) {
    socklen_t lg;
    SOCKADDR_IN p_adresse;
    HOSTENT * h = gethostbyname(host);
    /* p_adresse.sin_addr.s_addr = inet_addr (host); */
    bcopy(h->h_addr, &p_adresse.sin_addr, h->h_length);
    p_adresse.sin_family = AF_INET;
    p_adresse.sin_port = htons(port);

    lg = (socklen_t) sizeof (SOCKADDR_IN);
    return recvfrom(sock, msg, sizeof (OBJET), 0, (SOCKADDR*) & p_adresse, &lg);
}

SOCKET recvFromUDP(SOCKET sock, OBJET * msg, char *host) {
    socklen_t lg;
    SOCKADDR_IN p_adresse;
    HOSTENT * h = gethostbyname(host);
    /* p_adresse.sin_addr.s_addr = inet_addr (host); */
    bcopy(h->h_addr, &p_adresse.sin_addr, h->h_length);
    p_adresse.sin_family = AF_INET;

    lg = (socklen_t) sizeof (SOCKADDR_IN);
    return recvfrom(sock, msg, sizeof (OBJET), 0, (SOCKADDR*) & p_adresse, &lg);
}

/*********** SOCKET TCP ***********/

/*** Partie commune ***/
SOCKET initSocketTCP(void) {
    return socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

SOCKET sendTCP(SOCKET sock_service, OBJET *obj) {
    return send(sock_service, obj, sizeof (OBJET), 0);
}

SOCKET recvTCP(SOCKET sock_service, OBJET *obj) {
    return recv(sock_service, obj, sizeof (OBJET), 0);
}

/*** Partie server ***/
SOCKET serverBindServerTCP(SOCKET sock, int port) {
    SOCKADDR_IN p_adresse;
    bzero(&p_adresse, sizeof (SOCKADDR_IN));
    /* ou memset (&p_asresse, 0, sizeof (SOCKADDR_IN));*/
    p_adresse.sin_addr.s_addr = INADDR_ANY;
    p_adresse.sin_family = AF_INET;
    p_adresse.sin_port = htons(port);

    return bind(sock, (SOCKADDR*) & p_adresse, sizeof (SOCKADDR_IN));
}

// En attente des connexions

SOCKET serverEcouteListenTCP(SOCKET sock, int nbmaxconnexion) {
    // nbmaxconnexion simultannée sur la socket d'écoute
    return listen(sock, nbmaxconnexion);
}

// retourne la socket tcp de service

SOCKET serverAcceptServiceTCP(SOCKET sock_ecoute) {
    SOCKADDR_IN p_adresse_client;
    socklen_t len;
    bzero(&p_adresse_client, sizeof (SOCKADDR_IN));
    /* ou memset (&p_adresse_client, 0, sizeof (SOCKADDR_IN));*/
    len = (socklen_t) sizeof (SOCKADDR_IN);
    return accept(sock_ecoute, (SOCKADDR*) & p_adresse_client, &len);
}

/*** Partie client ***/

SOCKET clientConnectServiceTCP(SOCKET sock_service, char *ipServer, int portServer) {
    SOCKADDR_IN p_adresse_server;
    socklen_t len;
    bzero(&p_adresse_server, sizeof (SOCKADDR_IN));
    p_adresse_server.sin_family = AF_INET;
    p_adresse_server.sin_port = htons(portServer);
    p_adresse_server.sin_addr.s_addr = inet_addr(ipServer);
    len = (socklen_t) sizeof (SOCKADDR_IN);

    return connect(sock_service, (SOCKADDR*) & p_adresse_server, len);
}

/*********** SOCKET UNIX *************/

/* Création de la file de message, allocation  MSQ */
int msgqCreate(int clef) {
    return msgget((key_t) clef, 0750 | IPC_CREAT | IPC_EXCL);
}

/*  Récupération  MSQ */
int msgqGet(int clef) {
    return msgget((key_t) clef, 0);
}

int msgqSend(int msqid, OBJET_LIGHT *msg) {
    /* -sizeof (long) pour la taille utile du message (on soustrait la taille de l'entête) */
    return msgsnd(msqid, msg, sizeof (OBJET_LIGHT) - sizeof (long), 0);
}

int msgqRecv(int msqid, OBJET_LIGHT *msg) {
    /* -sizeof(long) pour la taille utile du message (on soustrait la taille de l'entête) */
    /* PID est l'entête, normalement on choisit le pid du destinataire, là c'est arbitraire */
    return msgrcv(msqid, msg, sizeof (OBJET_LIGHT) - sizeof (long), PID, 0);
}

int semCreate(int clef) {
    return semget((key_t) clef, 1, IPC_CREAT | IPC_EXCL | 0600);
}

/* Init(semid, 1) */
int semInit(int semid) {
    return semctl(semid, 0, SETVAL, 1);
}

int semClose(int semid) {
    return semctl(semid, 0, IPC_RMID, 0);
}

/* P(semid) => verouille */
void semP(int semid) {
    SEMBUF operation;
    operation.sem_num = 0;
    operation.sem_op = -1;
    operation.sem_flg = 0;
    semop(semid, &operation, 1);
}

/* V(semid) => deverouille */
void semV(int semid) {
    SEMBUF operation;
    operation.sem_num = 0;
    operation.sem_op = 1;
    operation.sem_flg = 0;
    semop(semid, &operation, 1);
}

/* ipcrm M (shared memory) / Q (msgq) / S (sem) */
void ipcDelete(int clef, char args) {
    char str[100];
    /* suppression de l'ipc au niveau shell avec redirection de STDIN et STDOUT dans /dev/null */
    sprintf(str, "ipcrm -%c %d > /dev/null 2>&1", args, clef);
    system(str);
}