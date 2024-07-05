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

//definizione funzioni

int CercaUtente (char* utente) {
    //implementare lista
    return 0;
}

//
void* asdrubale (void* arg) {
    //
    int fd_client = *(int*) arg;
    char type;

    //
    char* input = Ade(fd_client, &type);

    //
    while ((type != MSG_REGISTRA_UTENTE && type != MSG_FINE) || (CercaUtente(input) == 0)) {
        //
        if (CercaUtente(input) == 0) 
            Caronte(fd_client, "Scrivi un altro nome utente: quello inviato è già stato preso\n", MSG_ERR);

        //
        Caronte(fd_client, "Errore: devi prima registrarti\n", MSG_ERR);
        //
        free(input);
        //
        input = Ade(fd_client, &type);
    }
    //
    if (type == MSG_FINE) {
        pthread_exit(NULL);
    }
    /*Funzione dove aggiungo in lista il nome utente*/

    free(input);
    //
    while(type != MSG_FINE && type != MSG_CANCELLA_UTENTE) {
        input = Ade(fd_client, &type);
        //QUI DENTRO IL CLIENT GIOCA
    }
    //
    if (type == MSG_CANCELLA_UTENTE) {
        //rimuovo dalla lista
        //pthread_exit
    }
    return NULL;
}

//
void* Argo(void* arg) {
    return NULL;
}

int main (int argc, char* argv[]) {
    // gestire errori per numero di parametri, ecc...

    //creo l'identificatore per il socket, salvo e casto come intero la porta del server
    int fd_server, porta_server = atoi(argv[2]), retvalue;
    pthread_t Cerbero;

    //salvo il nome del server
    char* nome_server = argv[1];

    //creo una struct con le informazioni del server
    struct sockaddr_in info_server;
    info_server.sin_family = AF_INET; 
    info_server.sin_port = htons(porta_server);
    info_server.sin_addr.s_addr = inet_addr(nome_server);

    //creo la socket del server e alloco il file descriptor del server
    SYSC(fd_server, socket(AF_INET, SOCK_STREAM, 0),"Errore Socket");

    //associo il file descriptor del server ad un indirizzo
    SYSC(retvalue, bind(fd_server, (struct sockaddr *)&info_server, sizeof(info_server)), "Errore Bind server");

    //siamo in ricevimento di altri procesi/client
    SYSC(retvalue, listen(fd_server, 32), "Errore listen server");

    //Creazione Cerbero -> ricordarsi/capire cosa fa
    SYST(retvalue, pthread_create(&Cerbero, NULL, Argo, NULL), "Errore creazione Cerbero");

    //creo ed associo un thread per ogni client che si connette al server, ogni client applica la funzione asdrubale
    while(1) {
        int fdtemp;
        pthread_t tidtemp;
        SYSC(fdtemp, accept(fd_server, NULL, 0), "Errore accept server");
        SYST(retvalue, pthread_create(&tidtemp, NULL, asdrubale, &fdtemp), "Errore pthread create collegato client");
    }

    //

    return 0;
}