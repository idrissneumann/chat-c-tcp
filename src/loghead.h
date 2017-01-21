#ifndef SOCKET_ERROR
#include "sockethead.h"
#endif

#define MAX_DATE 20
#ifndef MAX_NAME
#define MAX_NAME 40
#endif

#define SERVER_LOG_FILE "server-%s.log"
#define CLIENT_LOG_FILE "client-%s.log"
#define CLIENT_SHELL_LOG_FILE "client_shell-%s.log"

/* levels */
#define LOG_DEBUG "LOG_DEBUG"
#define LOG_ERR "LOG_ERR"
#define LOG_WARNING "LOG_WARNING"
#define LOG_INFO "LOG_INFO"

void getDateExtension (char*);
void getDateSystem (char*);
FILE* initLogFile(char*);
void logMessage(FILE*, char*, char*);
void logSend(FILE*, OBJET);
void logSendTo(FILE*, char*, OBJET);
void logBroadcast(FILE*, OBJET);
void logRecv(FILE*, OBJET);
void logWarning(FILE*, OBJET);
void logThread(FILE*, SOCKET);