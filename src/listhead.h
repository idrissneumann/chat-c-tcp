#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#define NOT_EXIST_EXCEPTION NULL
#define MAX_ID 200

struct slist {
    char id[MAX_ID];
    int state;
    int descripteurSocket;
    struct slist* next;
};

typedef struct slist LIST;

LIST* getElementById(char*, LIST*);
LIST* getElementByDescriptor(int, LIST*);
int in_list_id (char*, LIST*);
LIST* new_id(char*);
LIST* new_elem(char*, int);
void add_id(LIST**, char*);
void add_elem(LIST**, char*, int);
void print_r(LIST*, int);