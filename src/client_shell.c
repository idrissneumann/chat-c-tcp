#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#include "sockethead.h"
#include "iohead.h"

#ifndef MAX_DATE
#include "loghead.h"
#endif

#ifndef NOT_EXIST_EXCEPTION
#include "listhead.h"
#endif

#define keyShm 123456
#define max_str 100

int main(void) {
    OBJET msg, last;
    int *quit = FALSE, shmId, retval;
    char ipServer[max_str], str[MAX_MSG], lastStr[MAX_MSG], port[max_str];
    pid_t pid;
    FILE *logFile, *myFile;
    LIST *listeFichiers = NULL, *fichier = NULL;

    strcpy(msg.type, TYPE_MSG);
    strcpy(last.type, TYPE_MSG);
    strcpy(lastStr, MSG_NULL);
    logFile = initLogFile(CLIENT_SHELL_LOG_FILE);

    printf("Veuillez choisir un pseudo : ");
    read_stdin(msg.pseudo, max_str);
    printf("Veuillez indiquer l'ip du server [%s] : ", HOST);
    read_stdin(ipServer, max_str);
    if (strlen(ipServer) < 2){
        printf ("Connexion sur le serveur %s\n", HOST);
        strcpy(ipServer, HOST);
    }
    printf ("Veuillez indiquer le port du serveur [%d] : ", PORT);
    read_stdin(port, max_str);
    if (strlen(port) < 2){
        printf ("Connexion sur le port %d\n", PORT);
        sprintf(port, "%d", PORT);
    }
    sprintf(msg.content, MSG_CONNECT);
    sprintf(last.content, MSG_NULL);

    /* initialisation de la socket */
    SOCKET maSocket = initSocketTCP();

    if (maSocket == SOCKET_ERROR) {
        logMessage(logFile, LOG_ERR, "Socket error !");
        fprintf(stderr, "Socket error !\n");
        return EXIT_FAILURE;
    }

    if (clientConnectServiceTCP(maSocket, ipServer, atoi(port)) == SOCKET_ERROR) {
        logMessage(logFile, LOG_ERR, "Connect error ! => serveur non trouve !");
        fprintf(stderr, "Connect error !\n");
        return EXIT_FAILURE;
    }

    /* cette premier send obligera le client à se déclarer auprès du server */
    logSend(logFile, msg);
    sendTCP(maSocket, &msg);
    logMessage(logFile, LOG_INFO, "envoye OK !");

    /* Création des segments de mémoire partagées */
    ipcDelete(keyShm, 'M');
    if ((shmId = shmget((key_t) quit, sizeof (int), 0666 | IPC_EXCL | IPC_CREAT)) <= IPC_ERROR) {
        logMessage(logFile, LOG_ERR, "shmget creat error !");
        fprintf(stderr, "shmget creat error !\n");
        return EXIT_FAILURE;
    }

    /* Attachement du pointeur au segment de mémoire partagée */
    if ((quit = shmat(shmId, NULL, 0)) == (int*) IPC_ERROR) {
        logMessage(logFile, LOG_ERR, "shmat error !");
        fprintf(stderr, "shmat error !\n");
        return EXIT_FAILURE;
    }

    logMessage(logFile, LOG_INFO, "fork");
    pid = fork();

    while (!*quit) {
        if (pid == 0) {
            retval = time_out(maSocket);
            if (retval) {
                recvTCP(maSocket, &msg);
                if (strcmp(msg.content, MSG_EXIT) != 0 && strcmp(msg.content, MSG_CONNECT) != 0) {
                    /* On ne repète pas un message identique au précédant */
                    if (strcmp(msg.type, TYPE_FILE) == 0) { /* reception de fichier */
                        if (strcmp(msg.content, MSG_EOF) != 0) {
                            if (!in_list_id(msg.nameFile, listeFichiers)) {
                                add_id(&listeFichiers, msg.nameFile);

                                /* on écrase le fichier */
                                myFile = fopen(msg.nameFile, "w");
                                fwrite(msg.content, sizeof(char), sizeof(msg.content)/sizeof(char), myFile);
                                fclose(myFile);
                            } else {
                                fichier = getElementById(msg.nameFile, listeFichiers);
                                if (fichier->state) {
                                    /* on continue à partir de la fin du fichier */
                                    myFile = fopen(msg.nameFile, "a");
                                } else {
                                    fichier->state = TRUE;
                                    /* on écrase le fichier */
                                    myFile = fopen(msg.nameFile, "w");
                                }

                                fwrite(msg.content, sizeof(char), sizeof(msg.content)/sizeof(char), myFile);
                                fclose(myFile);
                            }

                            sprintf(str, "From : %s part of %s", msg.pseudo, fichier->id);
                            logMessage(logFile, LOG_INFO, str);
                            strcpy(lastStr, MSG_NULL);
                        } else {
                            fichier = getElementById(msg.nameFile, listeFichiers);
                            if (fichier != NOT_EXIST_EXCEPTION) {
                                /* désactivation du fichier pour ne pas recommencer à la suite au prochain envoie */
                                fichier->state = FALSE;
                            }
                            sprintf(str, "%s a finit l'envoie du fichier %s", msg.pseudo, fichier->id);
                            if (strcmp(str, lastStr) != 0){
                                printf ("%s\n", str);
                                logMessage(logFile, LOG_INFO, str);
                                strcpy(lastStr, str);
                            }
                        }
                    } else if ((strcmp(msg.content, last.content) != 0 && strcmp(msg.content, MSG_HELLO)) || strcmp(msg.content, MSG_KILL) == 0) {
                        logRecv(logFile, msg);
                        if (strcmp(msg.content, MSG_KILL) == 0) {
                            sprintf(msg.content, "%s vient de quitter le chat", msg.pseudo);
                        }
                        printf("\t[%s] : %s\n", msg.pseudo, msg.content);
                        last = msg;
                    }
                } else {
                    logMessage(logFile, LOG_WARNING, "pseudo deja utilise");
                    printf("Veuillez choisir un autre pseudo !\n\nAppuyez sur ENTER pour continuer...\n");
                    *quit = TRUE;
                }
            }
        } else { /* parent */
            read_stdin(msg.content, 100);

            /* Envoi au server */
            logSend(logFile, msg);
            sendTCP(maSocket, &msg);
            logMessage(logFile, LOG_INFO, "envoye OK !");

            if (strcmp(msg.content, MSG_EXIT) == 0) {
                printf("Bye !\n");
                *quit = TRUE;
            }
        }
    }

    /* Attente des fils */
    wait(NULL);

    /* détachement de la variables au segment de mémoire partagée */
    shmdt(quit);

    /* suppression du segment de mémoire partagée */
    shmctl(shmId, IPC_RMID, NULL);

    close(maSocket);

    fclose(logFile);

    return EXIT_SUCCESS;
}