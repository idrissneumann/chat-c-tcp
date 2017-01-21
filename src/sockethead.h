#include <stdio.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#define SOCKET_ERROR -1
#define IPC_ERROR -1
#define PORT 1111
#define STRPORT "1111"
#define MAX_MSG 200 /* la limite du champ du client est de 300 */
#ifndef MAX_NAME
#define MAX_NAME 50
#endif
#define MAX_TYPE 5
#define HOST "127.0.0.1"
#define SOCKET_TIME_OUT 60
#define PID 16 /* valeur arbitraire */
#define SEM_CLE 21212 /* valeur arbitraire */
#define SEM_ERROR -1
#define NB_MAX_CONNEXION_SIMULTANNEE 10

#define MSG_EXIT "/QUIT"
#define MSG_KILL "/KILL"
#define MSG_CONNECT "/CONNECT"
#define MSG_NULL "/NULL"
#define MSG_STOP "/STOP"
#define MSG_HELLO "/HELLO"
#define MSG_EOF "/EOF"

#define TYPE_FILE "FILE"
#define TYPE_MSG "MSG"
#define PSEUDO_SERVER "SERVER"
#define PSEUDO_DEFAULT "PSEUDO_INCONNU"

struct objet {
    char pseudo[MAX_NAME]; /* envoyeur */
    char content[MAX_MSG]; /* contenu du message (ou d'une partie d'un fichier */
    char nameFile[MAX_MSG]; /* nom du fichier envoyé (valorisé que si num > 1) */
    char dest[MAX_NAME]; /* destinataire du paquet (valorisé que si num > 1) */
    char type[MAX_TYPE]; /* type de message */
};

struct objet_light {
    char content[MAX_MSG];
};

typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;
typedef struct hostent HOSTENT;
typedef struct objet OBJET;
typedef struct objet_light OBJET_LIGHT;
typedef struct sembuf SEMBUF;

int time_out(SOCKET);

/*** UDP ***/
SOCKET initSocketUDP(void);
SOCKET bindSocketUDP(SOCKET, int);
SOCKET sendToUDP(SOCKET, OBJET*, char*, int);
SOCKET recvFromPUDP(SOCKET, OBJET*, char*, int);
SOCKET recvFromUDP(SOCKET, OBJET*, char*);
SOCKET recvSocketUDP(SOCKET, OBJET*);

/*** TCP ***/
SOCKET initSocketTCP(void);
SOCKET sendTCP(SOCKET, OBJET*);
SOCKET recvTCP(SOCKET, OBJET*);
SOCKET serverBindServerTCP(SOCKET, int);
SOCKET serverEcouteListenTCP(SOCKET, int);
SOCKET serverAcceptServiceTCP(SOCKET);
SOCKET clientConnectServiceTCP(SOCKET, char*, int);

/*** Unix ***/
int msgqCreate(int);
int msgqGet(int);
int msgqSend(int, OBJET_LIGHT*);
int msgqRecv(int, OBJET_LIGHT*);
int semCreate(int);
int semInit(int);
int semClose(int);
void semP(int);
void semV(int);
void ipcDelete(int, char);