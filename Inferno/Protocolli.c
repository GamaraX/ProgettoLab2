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
#include "../Purgatorio/macro.h"
#include "../Purgatorio/Protocolli.h"

//Definizione funzione che invia messaggi
int Caronte(int fd, char* msg, char carattere) {
    int len = strlen(msg), retvalue;
    SYSC(retvalue, write(fd, &len, sizeof(int)), "Errore in write lunghezza messaggio");
    SYSC(retvalue, write(fd, &carattere, sizeof(char)), "Errore in write carattere");
    SYSC(retvalue, write(fd, msg, len), "Errore write messaggio stesso");

    return 0;
}

//Definizione funzione che riceve messaggi
char* Ade(int fd, char* carattere) {
    int retvalue, length;
    
    SYSC(retvalue, read(fd, &length, sizeof(int)), "Errore in read lunghezza messaggio");
    SYSC(retvalue, read(fd, carattere, sizeof(char)), "Errore in read lunghezza messaggio");
    char* mess = (char*) malloc(length* sizeof(char));
    SYSC(retvalue, read(fd, mess, length), "Errore in read lunghezza messaggio");
    return mess;
}

