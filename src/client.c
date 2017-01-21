#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <gtk/gtk.h>
#include <glib.h>

#include "sockethead.h"
#include "iohead.h"
#ifndef NOT_EXIST_EXCEPTION
#include "listhead.h"
#endif
#ifndef MAX_DATE
#include "loghead.h"
#endif

/* Variables globales pour interface graphique */
GtkWidget *window;
GtkWidget *table, *table_connect, *table_msgs, *table_file, *scroll_tview, *scroll_tview_users;
GtkWidget *frame_connect, *frame_msgs, *frame_users, *frame_file;
GtkWidget *label_adr, *entry_adr, *label_port, *entry_port, *label_login, *entry_login, *label_msg, *entry_msg, *label_file_user, *entry_file_user, *entry_filename;
GtkWidget *button_connect, *button_send, *button_file, *button_browse, *button_quit;
GtkWidget *tview, *tview_users;
GtkWidget *file_dialog;

GtkEntryBuffer *entry_adr_buf, *entry_port_buf, *entry_login_buf, *entry_msg_buf, *entry_file_user_buf, *entry_filename_buf;
GtkTextBuffer *tview_buffer, *tview_users_buffer;
GtkTextIter iter, iter2;
gchar *file_name;

/* prototypes */
int confirm(char*);
void* threadClient(void*);
void deleteListeUser(void);
void clear(void);
void fonction_connect(void);
void affiche_users(gchar*);
void affiche_msg(gchar*, gchar*);
void fonction_envoie(void);
void fonction_envoie_eichier(void);
void fonction_quit(void);
void file_ok_sel(GtkWidget*, GtkFileSelection*);
void fonction_affich_fselect(void);
void aff_interface(int, char *argv[]);
void fileNameSplit(char*);

int quit = FALSE, isConnected = FALSE, portServer;
OBJET myMsg, msgRecv, last, info;
char ipServer[100];
SOCKET maSocket;
LIST *listeClients = NULL, *listeFichiers = NULL;
FILE *logFile;

int main(int argc, char *argv[]) {
    pthread_t thread;

    /* initialisation du fichier de log */
    logFile = initLogFile(CLIENT_LOG_FILE);

    /* pré-initialisations */
    strcpy(info.pseudo, "INFO");
    strcpy(info.content, MSG_NULL);
    strcpy(last.content, MSG_NULL);
    strcpy(info.type, TYPE_MSG);
    strcpy(last.type, TYPE_MSG);
    strcpy(myMsg.type, TYPE_MSG);

    /* thread ecoute du server */
    logMessage(logFile, LOG_INFO, "creation de la thread d'ecoute pour le client");
    pthread_create(&thread, NULL, threadClient, (void*) NULL);

    /* interface */
    logMessage(logFile, LOG_INFO, "ouverture de l'interface");
    aff_interface(argc, argv);
    fclose(logFile);
    return EXIT_SUCCESS;
}

int confirm(char *msg) {
    GtkWidget *dialog;
    gint resp;

    dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, (const gchar *) msg);

    resp = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    return (resp == GTK_RESPONSE_YES) ? TRUE : FALSE;
}

/* Fonction appelée si clique bouton Connexion */
void fonction_connect(void) {
    if (!isConnected) {
        strcpy(ipServer, (char*) gtk_entry_buffer_get_text(GTK_ENTRY_BUFFER(entry_adr_buf)));
        portServer = (int) g_ascii_strtod(gtk_entry_buffer_get_text(GTK_ENTRY_BUFFER(entry_port_buf)), NULL);
        strcpy(myMsg.pseudo, (char*) gtk_entry_buffer_get_text(GTK_ENTRY_BUFFER(entry_login_buf)));
        strcpy(myMsg.content, MSG_CONNECT);

        maSocket = initSocketTCP();

        if (maSocket == SOCKET_ERROR) {
            logMessage(logFile, LOG_ERR, "Socket error !");
            fprintf(stderr, "Socket error !\n");
            exit(1);
        }

        if (clientConnectServiceTCP(maSocket, ipServer, portServer) == SOCKET_ERROR) {
            logMessage(logFile, LOG_ERR, "Connect error ! => serveur non trouve !");
            fprintf(stderr, "Connect error !\n");
            exit(1);
        }

        logSend(logFile, myMsg);
        sendTCP(maSocket, &myMsg);
        logMessage(logFile, LOG_INFO, "envoye OK !");
        isConnected = TRUE;
    } else {
        strcpy(info.content, "connexion en cours !");
        if (strcmp(info.content, last.content) != 0) {
            logWarning(logFile, info);
            affiche_msg((gchar*) info.pseudo, (gchar*) info.content);
            last = info;
        }
    }
}

void* threadClient(void* arg) {
    int retval;
    LIST *client = NULL, *parcours = NULL, *fichier = NULL;
    FILE* myFile;
    char str[MAX_MSG];

    sprintf(last.content, MSG_NULL);

    while (!quit) {
        if (isConnected) {
            retval = time_out(maSocket);

            if (retval) {
                recvTCP(maSocket, &msgRecv);

                if (strcmp(msgRecv.content, MSG_EXIT) != 0) {
                    /* On ne repète pas un message identique au précédant */
                    if ((strcmp(msgRecv.content, last.content) != 0
                            && strcmp(msgRecv.content, MSG_CONNECT) != 0)
                            || strcmp(msgRecv.content, MSG_KILL) == 0
                            || strcmp(msgRecv.type, TYPE_FILE) == 0) {
                        if (strcmp(msgRecv.type, TYPE_FILE) == 0) { /* reception de fichier */
                            if (strcmp(msgRecv.content, MSG_EOF) != 0) {
                                if (!in_list_id(msgRecv.nameFile, listeFichiers)) {
                                    add_id(&listeFichiers, msgRecv.nameFile);

                                    /* on écrase le fichier */
                                    myFile = fopen(msgRecv.nameFile, "w");

                                    fwrite(msgRecv.content, sizeof (char), sizeof (msgRecv.content) / sizeof (char), myFile);
                                    fclose(myFile);
                                } else {
                                    fichier = getElementById(msgRecv.nameFile, listeFichiers);

                                    if (fichier->state) {
                                        /* on continue à partir de la fin du fichier */
                                        myFile = fopen(msgRecv.nameFile, "a");
                                    } else {
                                        fichier->state = TRUE;

                                        /* on écrase le fichier */
                                        myFile = fopen(msgRecv.nameFile, "w");
                                    }

                                    fwrite(msgRecv.content, sizeof (char), sizeof (msgRecv.content) / sizeof (char), myFile);
                                    fclose(myFile);
                                }

                                sprintf(str, "From : %s part of %s", msgRecv.pseudo, fichier->id);
                                logMessage(logFile, LOG_INFO, str);
                            } else {
                                fichier = getElementById(msgRecv.nameFile, listeFichiers);
                                if (fichier != NOT_EXIST_EXCEPTION) {
                                    /* désactivation du fichier pour ne pas recommencer à la suite au prochain envoie */
                                    fichier->state = FALSE;
                                }
                                sprintf(info.content, "%s a finit l'envoie du fichier %s", msgRecv.pseudo, fichier->id);
                                if (strcmp(last.content, info.content) != 0) {
                                    logRecv(logFile, info);
                                    affiche_msg((gchar*) info.pseudo, (gchar*) info.content);
                                    last = info;
                                }
                            }
                        } else if (strcmp(msgRecv.content, MSG_KILL) == 0) {
                            logRecv(logFile, msgRecv);
                            sprintf(msgRecv.content, "%s vient de quitter le chat", msgRecv.pseudo);
                            client = getElementById(msgRecv.pseudo, listeClients);
                            if (client != NOT_EXIST_EXCEPTION) {
                                client->state = FALSE;
                            }

                            deleteListeUser();

                            parcours = listeClients;
                            while (parcours != NULL) {
                                if (parcours->state) {
                                    affiche_users((gchar*) parcours->id);
                                }
                                parcours = parcours->next;
                            }
                        } else if (strcmp(msgRecv.pseudo, PSEUDO_SERVER) != 0
                                && !in_list_id(msgRecv.pseudo, listeClients)) {
                            logRecv(logFile, msgRecv);
                            add_id(&listeClients, msgRecv.pseudo);
                            affiche_users((gchar*) msgRecv.pseudo);
                        } else {
                            logRecv(logFile, msgRecv);
                            client = getElementById(msgRecv.pseudo, listeClients);
                            if (client != NOT_EXIST_EXCEPTION && !client->state) {
                                client->state = TRUE;
                                affiche_users((gchar*) client->id);
                            }
                        }

                        if (strcmp(msgRecv.content, MSG_CONNECT) != 0
                                && strcmp(msgRecv.content, MSG_HELLO) != 0
                                && strcmp(msgRecv.type, TYPE_FILE) != 0) {
                            affiche_msg((gchar*) msgRecv.pseudo, (gchar*) msgRecv.content);
                        }
                        last = msgRecv;
                    }
                } else {
                    strcpy(info.content, "Veuillez choisir un autre pseudo !");
                    isConnected = FALSE;
                    if (strcmp(info.content, last.content) != 0) {
                        logWarning(logFile, info);
                        affiche_msg((gchar*) info.pseudo, (gchar*) info.content);
                        last = info;
                    }
                }
            }
        }
    }
    return (void*) EXIT_SUCCESS;
}

void deleteListeUser(void) {
    GtkTextIter start;
    GtkTextIter end;
    gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(tview_users_buffer), &start, &end);
    gtk_text_buffer_delete(GTK_TEXT_BUFFER(tview_users_buffer), &start, &end);
}

void clear(void) {
    if (confirm("Souhaitez-vous vraiment tout effacer ?")) {
        GtkTextIter start;
        GtkTextIter end;
        gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(tview_buffer), &start, &end);
        gtk_text_buffer_delete(GTK_TEXT_BUFFER(tview_buffer), &start, &end);
    }
}

/* Fonction permettant affichage nouvel utilisateur connecté */
void affiche_users(gchar *login) {
    const gchar *user_display;

    /* Reconstitue info à afficher */
    user_display = g_strconcat(login, "\n", NULL);
    /* Affichage du utilisateur */
    gtk_text_buffer_insert_at_cursor(GTK_TEXT_BUFFER(tview_users_buffer), user_display, strlen(user_display));
    /* Force déroulement jusqu'à la dernière ligne */
    gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(tview_users_buffer), &iter2);
    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(tview_users), &iter2, 0.0, FALSE, 0, 0);
}

/* Fonction permettant affichage message sur espace Communication */
void affiche_msg(gchar *user, gchar *msg) {
    const gchar *msg_display;

    /* Reconstitue message à afficher */
    msg_display = g_strconcat("[", user, "] : ", msg, "\n", NULL);
    /* Affichage du message */
    gtk_text_buffer_insert_at_cursor(GTK_TEXT_BUFFER(tview_buffer), msg_display, strlen(msg_display));
    /* Force déroulement jusqu'au message le plus récent */
    gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(tview_buffer), &iter);
    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(tview), &iter, 0.0, FALSE, 0, 0);
}

/* Fonction appelée si clique bouton Envoyer (message) */
void fonction_envoie(void) {
    if (isConnected) {
        strcpy(myMsg.content, gtk_entry_buffer_get_text(GTK_ENTRY_BUFFER(entry_msg_buf)));
        logSend(logFile, myMsg);
        sendTCP(maSocket, &myMsg);
        logMessage(logFile, LOG_INFO, "envoye OK !");

        /* Supprime le message dans l'entrée Message */
        gtk_entry_buffer_delete_text(GTK_ENTRY_BUFFER(entry_msg_buf), 0, strlen(myMsg.content));

        if (strcmp(myMsg.content, MSG_EXIT) == 0) {
            fonction_quit();
        }
    } else {
        strcpy(info.content, "Veuillez vous connecter !");
        if (strcmp(info.content, last.content) != 0) {
            logWarning(logFile, info);
            affiche_msg((gchar*) info.pseudo, (gchar*) info.content);
            last = info;
        }
    }
}

/* Fonction appelée si clique bouton Envoyer fichier */
void fonction_envoie_fichier(void) {
    FILE *sendFile;
    OBJET msgFile;
    char str[MAX_MSG];

    strcpy(msgFile.nameFile, gtk_entry_buffer_get_text(GTK_ENTRY_BUFFER(entry_filename_buf)));
    strcpy(msgFile.dest, gtk_entry_buffer_get_text(GTK_ENTRY_BUFFER(entry_file_user_buf)));
    strcpy(msgFile.pseudo, myMsg.pseudo);
    strcpy(msgFile.type, TYPE_FILE);

    if (isConnected) {
        if (strlen(msgFile.nameFile) >= 2 && strlen(msgFile.dest) >= 2) {
            sendFile = fopen(msgFile.nameFile, "r");

            fileNameSplit(msgFile.nameFile);

            if (sendFile != NULL) {
                while (fread(msgFile.content, sizeof (char), (sizeof (msgFile.content) / sizeof (char)), sendFile)) {
                    sprintf(str, "Part of %s send to %s", msgFile.nameFile, msgFile.dest);
                    logMessage(logFile, LOG_INFO, str);
                    sendTCP(maSocket, &msgFile);
                    memset(msgFile.content, 0, sizeof (msgFile.content));
                }

                fclose(sendFile);

                sprintf(msgFile.content, MSG_EOF);
                if (strcmp(msgFile.content, last.content) != 0) {
                    sendTCP(maSocket, &msgFile);
                    last = msgFile;
                }

                sprintf(info.content, "File %s send to %s", msgFile.nameFile, msgFile.dest);
                if (strcmp(info.content, last.content) != 0) {
                    logMessage(logFile, LOG_INFO, info.content);
                    affiche_msg((gchar*) info.pseudo, (gchar*) info.content);
                    last = info;
                }
            } else {
                strcpy(info.content, "Fichier invalide !");
                if (strcmp(info.content, last.content) != 0) {
                    logWarning(logFile, info);
                    affiche_msg((gchar*) info.pseudo, (gchar*) info.content);
                    last = info;
                }
            }
        } else {
            strcpy(info.content, "Veuillez saisir un destinataire et selectionner un fichier !");
            if (strcmp(info.content, last.content) != 0) {
                logWarning(logFile, info);
                affiche_msg((gchar*) info.pseudo, (gchar*) info.content);
                last = info;
            }
        }
    } else {
        strcpy(info.content, "Veuillez vous connecter !");
        if (strcmp(info.content, last.content) != 0) {
            logWarning(logFile, info);
            affiche_msg((gchar*) info.pseudo, (gchar*) info.content);
            last = info;
        }
    }
}

void fonction_quit(void) {
    if (confirm("Souhaitez-vous vraiment quitter ?")) {
        quit = TRUE;
        if (isConnected) {
            /* On indique la deconnexion au serveur */
            strcpy(myMsg.content, MSG_EXIT);
            logSend(logFile, myMsg);
            sendTCP(maSocket, &myMsg);
            logMessage(logFile, LOG_INFO, "envoye OK !");
        }
        close(maSocket);
        logMessage(logFile, LOG_INFO, "exit");
        exit(0);
    }
}

/* Fonction récupérant fichier sélectionné si appuie bouton OK  */
void file_ok_sel(GtkWidget *w, GtkFileSelection *fs) {
    /* Récupère le nom du fichier séléctionné par l'utilisateur */
    file_name = (char *) gtk_file_selection_get_filename(GTK_FILE_SELECTION(fs));
    gtk_widget_destroy(file_dialog);
    gtk_entry_buffer_set_text(entry_filename_buf, file_name, strlen(file_name));
}

/* Affichage boite dialogue selection fichier si appuie bouton Parcourir */
void fonction_affich_fSelect(void) {
    /* Création de la boite de dialogue pour selection fichier */
    file_dialog = gtk_file_selection_new("Choix du fichier");
    g_signal_connect(file_dialog, "destroy", G_CALLBACK(gtk_widget_destroy), NULL);
    /* Connecte bouton Ok à la fonction file_ok_sel */
    g_signal_connect(GTK_FILE_SELECTION(file_dialog)->ok_button, "clicked", G_CALLBACK(file_ok_sel), (gpointer) file_dialog);
    /* Connecte bouton Cancel à la fonction de destruction du widget */
    g_signal_connect_swapped(GTK_FILE_SELECTION(file_dialog)->cancel_button, "clicked", G_CALLBACK(gtk_widget_destroy), file_dialog);
    gtk_widget_show(file_dialog);
}

/* Fonction créant interface client */
void aff_interface(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);

    gtk_window_set_title(GTK_WINDOW(window), "Client pour chat");
    gtk_container_set_border_width(GTK_CONTAINER(window), 15);

    table = gtk_table_new(3, 2, FALSE);
    gtk_table_set_col_spacings(GTK_TABLE(table), 10);
    gtk_table_set_row_spacings(GTK_TABLE(table), 5);

    frame_connect = gtk_frame_new("Connexion au serveur");
    gtk_frame_set_shadow_type(GTK_FRAME(frame_connect), GTK_SHADOW_ETCHED_IN);
    gtk_table_attach(GTK_TABLE(table), frame_connect, 0, 1, 0, 1, 0, 0, 0, 0);
    table_connect = gtk_table_new(2, 4, FALSE);
    gtk_table_set_col_spacings(GTK_TABLE(table_connect), 10);
    gtk_table_set_row_spacings(GTK_TABLE(table_connect), 5);
    gtk_container_set_border_width(GTK_CONTAINER(table_connect), 10);
    gtk_container_add(GTK_CONTAINER(frame_connect), table_connect);

    label_adr = gtk_label_new("Adresse :");
    entry_adr = gtk_entry_new();
    entry_adr_buf = gtk_entry_buffer_new(HOST, -1);
    gtk_entry_buffer_set_max_length(GTK_ENTRY_BUFFER(entry_adr_buf), 15);
    gtk_entry_set_buffer(GTK_ENTRY(entry_adr), GTK_ENTRY_BUFFER(entry_adr_buf));

    label_port = gtk_label_new("Port :");
    entry_port = gtk_entry_new();
    entry_port_buf = gtk_entry_buffer_new(STRPORT, -1);
    gtk_entry_buffer_set_max_length(GTK_ENTRY_BUFFER(entry_port_buf), 5);
    gtk_entry_set_buffer(GTK_ENTRY(entry_port), GTK_ENTRY_BUFFER(entry_port_buf));

    gtk_table_attach_defaults(GTK_TABLE(table_connect), label_adr, 0, 1, 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(table_connect), entry_adr, 1, 2, 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(table_connect), label_port, 2, 3, 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(table_connect), entry_port, 3, 4, 0, 1);

    label_login = gtk_label_new("Login :");
    entry_login = gtk_entry_new();
    entry_login_buf = gtk_entry_buffer_new(NULL, -1);
    gtk_entry_buffer_set_max_length(GTK_ENTRY_BUFFER(entry_login_buf), 50);
    gtk_entry_set_buffer(GTK_ENTRY(entry_login), GTK_ENTRY_BUFFER(entry_login_buf));

    button_connect = gtk_button_new_with_label("Connexion");
    g_signal_connect(G_OBJECT(button_connect), "clicked", G_CALLBACK(fonction_connect), NULL);

    button_quit = gtk_button_new_with_label("Quitter");
    g_signal_connect(G_OBJECT(button_quit), "clicked", G_CALLBACK(fonction_quit), NULL);

    gtk_table_attach_defaults(GTK_TABLE(table_connect), label_login, 0, 1, 1, 2);
    gtk_table_attach_defaults(GTK_TABLE(table_connect), entry_login, 1, 2, 1, 2);
    gtk_table_attach_defaults(GTK_TABLE(table_connect), button_connect, 3, 4, 1, 2);
    gtk_table_attach_defaults(GTK_TABLE(table_connect), button_quit, 4, 5, 1, 2);

    frame_msgs = gtk_frame_new("Communication");
    gtk_frame_set_shadow_type(GTK_FRAME(frame_msgs), GTK_SHADOW_ETCHED_IN);
    gtk_table_attach_defaults(GTK_TABLE(table), frame_msgs, 0, 1, 1, 2);
    table_msgs = gtk_table_new(2, 3, FALSE);
    gtk_table_set_col_spacings(GTK_TABLE(table_msgs), 10);
    gtk_table_set_row_spacings(GTK_TABLE(table_msgs), 5);
    gtk_container_set_border_width(GTK_CONTAINER(table_msgs), 10);
    gtk_container_add(GTK_CONTAINER(frame_msgs), table_msgs);

    scroll_tview = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(GTK_WIDGET(scroll_tview), 500, 300);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_tview), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
    tview = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(tview), GTK_WRAP_WORD);
    tview_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(tview));
    gtk_text_buffer_create_tag(tview_buffer, "gap", "pixels_above_lines", 30, NULL);
    gtk_text_buffer_create_tag(tview_buffer, "lmarg", "left_margin", 5, NULL);
    gtk_text_buffer_get_iter_at_offset(tview_buffer, &iter, 0);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(tview), 10);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(tview), 10);
    gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(tview), 10);
    gtk_text_view_set_pixels_below_lines(GTK_TEXT_VIEW(tview), 10);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(tview), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(tview), FALSE);
    gtk_container_add(GTK_CONTAINER(scroll_tview), tview);
    gtk_table_attach_defaults(GTK_TABLE(table_msgs), scroll_tview, 0, 4, 0, 1);

    label_msg = gtk_label_new("Message :");
    entry_msg = gtk_entry_new();
    gtk_widget_set_size_request(GTK_WIDGET(entry_msg), 400, 30);
    entry_msg_buf = gtk_entry_buffer_new(NULL, -1);
    gtk_entry_buffer_set_max_length(GTK_ENTRY_BUFFER(entry_msg_buf), MAX_MSG);
    gtk_entry_set_buffer(GTK_ENTRY(entry_msg), GTK_ENTRY_BUFFER(entry_msg_buf));

    button_send = gtk_button_new_with_label("Envoyer");
    g_signal_connect(G_OBJECT(button_send), "clicked", G_CALLBACK(fonction_envoie), NULL);
    gtk_table_attach_defaults(GTK_TABLE(table_msgs), label_msg, 0, 1, 1, 2);
    gtk_table_attach_defaults(GTK_TABLE(table_msgs), entry_msg, 1, 2, 1, 2);
    gtk_table_attach(GTK_TABLE(table_msgs), button_send, 2, 3, 1, 2, GTK_FILL, 0, 0, 0);

    button_quit = gtk_button_new_with_label("Effacer");
    g_signal_connect(G_OBJECT(button_quit), "clicked", G_CALLBACK(clear), NULL);
    gtk_table_attach(GTK_TABLE(table_msgs), button_quit, 3, 4, 1, 2, GTK_FILL, 0, 0, 0);

    frame_users = gtk_frame_new("Utilisateurs");
    gtk_frame_set_shadow_type(GTK_FRAME(frame_users), GTK_SHADOW_ETCHED_IN);
    gtk_table_attach_defaults(GTK_TABLE(table), frame_users, 1, 2, 1, 2);

    scroll_tview_users = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(GTK_WIDGET(scroll_tview_users), 190, 300);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_tview_users), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
    tview_users = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(tview), GTK_WRAP_WORD);
    tview_users_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(tview_users));
    gtk_text_buffer_create_tag(tview_users_buffer, "gap", "pixels_above_lines", 30, NULL);
    gtk_text_buffer_create_tag(tview_users_buffer, "lmarg", "left_margin", 5, NULL);
    gtk_text_buffer_get_iter_at_offset(tview_users_buffer, &iter, 0);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(tview_users), 10);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(tview_users), 10);
    gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(tview_users), 10);
    gtk_text_view_set_pixels_below_lines(GTK_TEXT_VIEW(tview_users), 10);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(tview_users), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(tview_users), FALSE);
    gtk_container_set_border_width(GTK_CONTAINER(tview_users), 5);
    gtk_container_add(GTK_CONTAINER(scroll_tview_users), tview_users);
    gtk_container_add(GTK_CONTAINER(frame_users), scroll_tview_users);

    frame_file = gtk_frame_new("Envoie de fichier");
    gtk_frame_set_shadow_type(GTK_FRAME(frame_file), GTK_SHADOW_ETCHED_IN);
    gtk_table_attach_defaults(GTK_TABLE(table), frame_file, 0, 1, 2, 3);
    table_file = gtk_table_new(1, 5, FALSE);
    gtk_table_set_col_spacings(GTK_TABLE(table_file), 10);
    gtk_table_set_row_spacings(GTK_TABLE(table_file), 5);
    gtk_container_set_border_width(GTK_CONTAINER(table_file), 10);
    gtk_container_add(GTK_CONTAINER(frame_file), table_file);

    entry_filename = gtk_entry_new();
    entry_filename_buf = gtk_entry_buffer_new(NULL, -1);
    gtk_entry_set_buffer(GTK_ENTRY(entry_filename), GTK_ENTRY_BUFFER(entry_filename_buf));

    button_browse = gtk_button_new_with_label("Parcourir");
    g_signal_connect(G_OBJECT(button_browse), "clicked", G_CALLBACK(fonction_affich_fSelect), NULL);

    label_file_user = gtk_label_new("Destinataire :");
    entry_file_user = gtk_entry_new();
    entry_file_user_buf = gtk_entry_buffer_new("pseudo", 6);
    gtk_entry_buffer_set_max_length(GTK_ENTRY_BUFFER(entry_file_user_buf), MAX_NAME);
    gtk_entry_set_buffer(GTK_ENTRY(entry_file_user), GTK_ENTRY_BUFFER(entry_file_user_buf));

    button_file = gtk_button_new_with_label("Envoyer fichier");
    g_signal_connect(G_OBJECT(button_file), "clicked", G_CALLBACK(fonction_envoie_fichier), NULL);
    gtk_table_attach(GTK_TABLE(table_file), entry_filename, 0, 1, 0, 1, GTK_EXPAND | GTK_FILL, 0, 0, 0);
    gtk_table_attach(GTK_TABLE(table_file), button_browse, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, 0, 0, 0);
    gtk_table_attach(GTK_TABLE(table_file), label_file_user, 2, 3, 0, 1, 0, 0, 0, 0);
    gtk_table_attach(GTK_TABLE(table_file), entry_file_user, 3, 4, 0, 1, 0, 0, 0, 0);
    gtk_table_attach(GTK_TABLE(table_file), button_file, 4, 5, 0, 1, 0, 0, 0, 0);

    gtk_container_add(GTK_CONTAINER(window), table);

    gtk_widget_show_all(window);

    g_signal_connect_swapped(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_main();
}

void fileNameSplit(char* name) {
    char tmp[MAX_MSG];
    int i, j = 0;
    for (i = strlen(name) - 1; i >= 0 && j < MAX_MSG && name[i] != '/'; i--) {
        tmp[j] = name[i];
        j++;
    }

    tmp[j] = '\0';
    j = 0;

    for (i = strlen(tmp) - 1; i >= 0 && j < MAX_MSG; i--) {
        name[j] = tmp[i];
        j++;
    }

    name[j] = '\0';
}