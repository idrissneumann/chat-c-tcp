#include <time.h>
#include <stdio.h>

#ifndef MAX_DATE
#include "loghead.h"
#endif

void getDateExtension (char *date) {
    time_t timestamp;
    struct tm *t;
    timestamp = time(NULL);
    t = localtime(&timestamp);
    sprintf (date, "%d_%d_%d_%d_%d_%d", t->tm_year, t->tm_mon, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
}

void getDateSystem (char *date) {
    time_t timestamp;
    struct tm *t;
    timestamp = time(NULL);
    t = localtime(&timestamp);
    sprintf (date, "%d/%d/%d:%d:%d:%d", t->tm_year, t->tm_mon, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
}

FILE* initLogFile(char *name){
    char date[MAX_DATE];
    char fname[MAX_NAME];
    getDateExtension(date);
    sprintf (fname, name, date);
    printf ("Creation du fichier de log : %s\n", fname);
    return fopen(fname, "w");
}

void logMessage(FILE *logFile, char *level, char *message){
    char date[MAX_DATE];
    if (logFile != NULL){
        getDateSystem(date);
        fprintf(logFile, "[%s][%s] %s\n", date, level, message);
    }
}

void logSend(FILE *logFile, OBJET msg){
    char date[MAX_DATE];
    if (logFile != NULL){
        getDateSystem(date);
        fprintf(logFile, "[%s][%s][%s][SEND %s] %s\n", date, LOG_INFO, msg.type, msg.pseudo, msg.content);
    }
}

void logSendTo(FILE *logFile, char* id, OBJET msg){
    char date[MAX_DATE];
    if (logFile != NULL){
        getDateSystem(date);
        fprintf(logFile, "[%s][%s][%s][SENDTO %s] %s : %s\n", date, LOG_INFO, msg.type, id, msg.pseudo, msg.content);
    }
}

void logBroadcast(FILE *logFile, OBJET msg){
    char date[MAX_DATE];
    if (logFile != NULL){
        getDateSystem(date);
        fprintf(logFile, "[%s][%s][%s][BROADCAST %s] %s\n", date, LOG_INFO, msg.type, msg.pseudo, msg.content);
    }
}

void logRecv(FILE *logFile, OBJET msg){
    char date[MAX_DATE];
    if (logFile != NULL){
        getDateSystem(date);
        fprintf(logFile, "[%s][%s][%s][RECV %s] %s\n", date, LOG_INFO, msg.type, msg.pseudo, msg.content);
    }
}

void logWarning(FILE *logFile, OBJET msg){
    char date[MAX_DATE];
    if (logFile != NULL){
        getDateSystem(date);
        fprintf(logFile, "[%s][%s][%s][%s] %s\n", date, LOG_WARNING, msg.type, msg.pseudo, msg.content);
    }
}

void logThread(FILE *logFile, SOCKET descriptor){
    char date[MAX_DATE];
    if (logFile != NULL){
        getDateSystem(date);
        fprintf(logFile, "[%s][%s] creation d'une thread pour la socketService %d\n", date, LOG_INFO, descriptor);
    }
}