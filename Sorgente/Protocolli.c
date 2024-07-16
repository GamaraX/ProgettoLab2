//Definizione Librerie
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <ctype.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "../Header/macro.h"
#include "../Header/Protocolli.h"
#include "../Header/Lista.h"
#include "../Header/Matrice.h"
#include "../Header/LogFun.h"

// Definizione funzione che invia messaggi
int Caronte(int fd, char* msg, char carattere){
    int len = strlen(msg), retvalue;
    // invio la lunghezza del messaggio come primo parametro
    SYSC(retvalue, write(fd, &len, sizeof(int)), "Errore in write lunghezza messaggio");
    // invio il tipo del messaggio come secondo parametro
    SYSC(retvalue, write(fd, &carattere, sizeof(char)), "Errore in write carattere");
    // infine invio il contenuto del messaggio
    SYSC(retvalue, write(fd, msg, len), "Errore write messaggio stesso");

    return 0;
}

// Definizione funzione che riceve messaggi
Msg* Ade(int fd){
    int retvalue, length;
    // alloco una struttura per memorizzare il messaggio
    Msg* msg = (Msg*)malloc(sizeof(Msg));
    // alloco il carattere pe memorizzare il carattere
    char* carattere = malloc(sizeof(char));
    //leggo il primo dato che Ã¨ la lunghezza del messaggio
    SYSC(retvalue, read(fd, &length, sizeof(int)), "Errore in read lunghezza messaggio");
    //alloco la stringa dove memorizzare il contenuto del messaggio 
    msg->msg = (char*)malloc(length + 1 * sizeof(char));
    //memorizzo il carattere contenente il tipo del messaggio
    SYSC(retvalue, read(fd, carattere, sizeof(char)), "Errore in read tipo messaggio");
    //memorizzo il contenuto del messaggio nella variabile allocata precedentemente
    SYSC(retvalue, read(fd, msg->msg, length), "Errore in read messaggio");
    //termino la stringa
    msg->msg[length] = '\0';
    //memorizzo la lunghezza nella struct del messaggio
    msg->length = length;
    //memorizzo il tipo di messaggio nella struct del messaggiol
    msg->type = carattere;
    return msg;
}


