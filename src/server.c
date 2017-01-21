#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#include "sockethead.h"

#ifndef NOT_EXIST_EXCEPTION
#include "listhead.h"
#endif

#ifndef MAX_DATE
#include "loghead.h"
#endif

void* threadClient(void*);
LIST *listeClients = NULL;
SOCKET sockEcoute;
FILE* logFile;

int main(void) {
    SOCKET sockService;
    pthread_t thread;
    int port_final;

    /* initialisation du fichier de log */
    logFile = initLogFile(SERVER_LOG_FILE);

    sockEcoute = initSocketTCP();

    if (sockEcoute == SOCKET_ERROR) {
        logMessage(logFile, LOG_ERR, "Socket error !");
        fprintf(stderr, "Socket error !\n");
        return EXIT_FAILURE;
    }
    
    port_final = PORT;
    while (serverBindServerTCP(sockEcoute, port_final) == SOCKET_ERROR){
        port_final++;
    }
    
    printf ("ATTENTION : Connexion sur le port : %d\n", port_final);

    if (serverEcouteListenTCP(sockEcoute, NB_MAX_CONNEXION_SIMULTANNEE)) {
        logMessage(logFile, LOG_ERR, "Listen error !");
        fprintf(stderr, "Listen error !\n");
        return EXIT_FAILURE;
    }

    logMessage(logFile, LOG_INFO, "Server is ready !");
    printf("Server is ready !\n");

    while (TRUE) {
        if ((sockService = serverAcceptServiceTCP(sockEcoute)) == -1) {
            logMessage(logFile, LOG_ERR, "Accept error !");
            fprintf(stderr, "Accept error !\n");
            return EXIT_FAILURE;
        }

        /* ajouter le descripteur de socket du nouveau client a une liste */
        add_elem(&listeClients, PSEUDO_DEFAULT, sockService);

        /* creer le thread */
        logThread(logFile, sockService);
        pthread_create(&thread, NULL, threadClient, (void*) sockService);
    }

    close(sockEcoute);
    fclose(logFile);
    return EXIT_SUCCESS;
}

void* threadClient(void *arg) {
    pthread_detach(pthread_self());
    int retval, vrecv, descriptor = (int*) arg;
    OBJET msg, last, hello;
    LIST *parcours = NULL, *dest = NULL, *client = NULL;
    
    strcpy(msg.type, TYPE_MSG);
    strcpy(last.type, TYPE_MSG);
    strcpy(hello.type, TYPE_MSG);
    
    sprintf(last.content, MSG_NULL);
    char info[MAX_MSG];
    
    client = getElementByDescriptor((int) descriptor, listeClients);

    do {
        retval = time_out((int) descriptor);
        vrecv = 50; /* valeur arbitraire pour re-initialiser */
        logMessage(logFile, LOG_INFO, "retval en attente ...");
        if (retval) {
            logMessage(logFile, LOG_INFO, "retval OK => recv ...");
            vrecv = recvTCP((int) descriptor, &msg);
            logRecv(logFile, msg);
            if (strcmp(msg.content, MSG_STOP) == 0) {
                close((int) descriptor);
                close(sockEcoute);
                fclose(logFile);
                exit(0);
            } else if ((vrecv == 0 || strcmp(msg.content, MSG_EXIT) == 0) && client->state) { /* déconnexion */
                client->state = FALSE;
                strcpy(client->id, PSEUDO_DEFAULT);
                strcpy(msg.content, MSG_KILL);
                
                /* envoi à tout les clients l'info comme quoi l'user est déconnecté */
                logBroadcast(logFile, msg);
                parcours = listeClients;
                while (parcours != NULL){
                    if (parcours->state){
                        logSendTo(logFile, parcours->id, msg);
                        sendTCP(parcours->descripteurSocket, &msg);
                        logMessage(logFile, LOG_INFO, "envoye OK !");
                    }
                    parcours = parcours->next;
                }
                logMessage(logFile, LOG_INFO, "broadcast OK !");
                last = msg;
            } else if (strcmp(client->id, PSEUDO_DEFAULT) == 0
                    && (strlen(msg.pseudo) < 2 || in_list_id(msg.pseudo, listeClients))) { /* Si le pseudo existe ou est vide */
                client->state = FALSE;
                strcpy(msg.content, MSG_EXIT);
                if (strcmp(msg.content, last.content) != 0) {
                    logSendTo(logFile, client->id, msg);
                    sendTCP((int) descriptor, &msg);
                    logMessage(logFile, LOG_INFO, "envoye OK !");
                    last = msg;
                }
            } else if (strcmp(client->id, PSEUDO_DEFAULT) == 0) { /* connexion */
                strcpy(client->id, msg.pseudo);
                /* envoie à tout le monde "machin s'est connecté" */
                sprintf(msg.content, "%s vient d'arriver sur le chat", msg.pseudo);
                if (strcmp(msg.content, last.content) != 0) {
                    parcours = listeClients;
                    while(parcours != NULL) {
                        if (parcours->state) {
                            logSendTo(logFile, parcours->id, msg);
                            sendTCP(parcours->descripteurSocket, &msg);
                            logMessage(logFile, LOG_INFO, "envoye OK !");

                            /* On envoie le message HELLO au client pour qu'il découvre les autres connectés */
                            if (strcmp(parcours->id, PSEUDO_DEFAULT) != 0) {
                                strcpy(hello.pseudo, parcours->id);
                                strcpy(hello.content, MSG_HELLO);
                                logSendTo(logFile, parcours->id, hello);
                                sendTCP((int) descriptor, &hello);
                                logMessage(logFile, LOG_INFO, "envoye OK !");
                            }
                        }
                        
                        parcours = parcours->next;
                    }
                    last = msg;
                }
            } else if (strcmp(msg.type, TYPE_FILE) == 0) { /* envoie de fichiers */
                if (in_list_id(msg.dest, listeClients)) {
                    dest = getElementById(msg.dest, listeClients);
                    if (dest->state) {
                        if (strcmp(msg.content, MSG_EOF) == 0) {
                            sprintf(info, "[EOF] %s a finit d'envoyer le fichier %s a %s", msg.pseudo, msg.nameFile, msg.dest);
                        } else {
                            sprintf(info, "%s envoie le fichier %s a %s", msg.pseudo, msg.nameFile, msg.dest);
                        }

                        logMessage(logFile, LOG_INFO, info);
                        sendTCP(dest->descripteurSocket, &msg);
                        last = msg;
                    } else {
                        strcpy(msg.pseudo, PSEUDO_SERVER);
                        strcpy(msg.type, TYPE_MSG);
                        sprintf(msg.content, "Utilisateur %s inexistant", msg.dest);
                        if (strcmp(msg.content, last.content) != 0) {
                            logSendTo(logFile, client->id, msg);
                            sendTCP((int) descriptor, &msg);
                            last = msg;
                        }
                    }
                } else {
                    strcpy(msg.pseudo, PSEUDO_SERVER);
                    strcpy(msg.type, TYPE_MSG);
                    sprintf(msg.content, "Utilisateur %s inexistant", msg.dest);
                    if (strcmp(msg.content, last.content) != 0) {
                        logSendTo(logFile, client->id, msg);
                        sendTCP((int) descriptor, &msg);
                        last = msg;
                    }
                }
            } else { /* message */
                if (strcmp(msg.content, last.content) != 0) {
                    logBroadcast(logFile, msg);
                    parcours = listeClients;
                    while(parcours != NULL) {
                        if (parcours->state) {
                            logSendTo(logFile, parcours->id, msg);
                            sendTCP(parcours->descripteurSocket, &msg);
                            logMessage(logFile, LOG_INFO, "envoye OK !");
                        }
                        parcours = parcours->next;
                    }
                    logMessage(logFile, LOG_INFO, "broadcast OK !");
                    last = msg;
                }
            }
        }
    } while (client->state);

    //fermeture de la socket
    close((int) descriptor);
    return (void*) EXIT_SUCCESS;
}

