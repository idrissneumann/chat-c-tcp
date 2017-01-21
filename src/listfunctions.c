#include<string.h>
#include<stdlib.h>
#include<stdio.h>

#ifndef NOT_EXIST_EXCEPTION
#include "listhead.h"
#include "sockethead.h"
#endif

/**
 * renvoie l'élèment dont l'id est passé en paramètre
 * @param id pseudo du client
 * @param array l de clients
 * @return l'id du client si le client existe, sinon NOT_EXIST_EXCEPTION
 */
LIST* getElementById(char *id, LIST *l) {
    while (l != NULL) {
        if (strcmp(l->id, id) == 0) {
            return l;
        }
        l = l->next;
    }

    return NOT_EXIST_EXCEPTION;
}

/**
 * renvoie l'élèment dont le descripteur de socket est passé en paramètre
 * @param id pseudo du client
 * @param array l de clients
 * @return l'id du client si le client existe, sinon NOT_EXIST_EXCEPTION
 */
LIST* getElementByDescriptor(int descripteur, LIST *l) {
    while (l != NULL) {
        if (l->descripteurSocket == descripteur) {
            return l;
        }
        l = l->next;
    }

    return NOT_EXIST_EXCEPTION;
}

/**
 * Test si un client existe dans la liste ou pas
 * @param id pseudo du client
 * @param l liste de clients
 * @return TRUE si le client existe, sinon false
 */
int in_list_id(char *id, LIST *l) {
    return (strcmp(id, PSEUDO_DEFAULT) == 0 || strcmp(id, PSEUDO_SERVER) == 0 || getElementById(id, l) != NOT_EXIST_EXCEPTION) ? TRUE : FALSE;
}

LIST *new_id(char *id){
    LIST *elem = malloc (sizeof(LIST));
    elem->next = NULL;
    elem->state = TRUE;
    strcpy(elem->id, id); 
    return elem;
}

LIST *new_elem(char *id, int descriptor){
    LIST *elem = new_id(id);
    elem->descripteurSocket = descriptor;
    return elem;
}

void add_id(LIST **l, char *id) {
    LIST *elem;
    if (*l == NULL){
        *l = new_id(id);
    } else {
        elem = *l;
        while (elem->next != NULL){
            elem = elem->next;
        }
        
        elem->next = new_id(id);
    }
}

void add_elem(LIST **l, char *id, int descriptor) {
    LIST *elem;
    
    if (*l == NULL){
        *l = new_elem(id, descriptor);
    } else {
        elem = *l;
        while (elem->next != NULL){
            elem = elem->next;
        }
        
        elem->next = new_elem(id, descriptor);
    }
}

void print_r(LIST *l, int affichDescriptor){
    printf ("List content :\n");
    while (l != NULL) {
        printf("\nid = %s, state = %d", l->id, l->state);
        if (affichDescriptor){
            printf(", descriptor = %d", l->descripteurSocket);
        }
        l = l->next;
    }
    printf ("\n\n");
}