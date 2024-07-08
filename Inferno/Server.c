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
#include "../Purgatorio/ListaClient.h"
#include "../Purgatorio/Matrice.h"
#include "../Purgatorio/Asdrubale.h"



//Definizione funzioni
void* asdrubale (void* arg) {
    //Fd del client
    int fd_client = *(int*) arg;
    Giocatore giocatore;
    printf("Connesso client su fd: %d\n", fd_client);

    Msg* msg;

    while (1) {
        msg = Ade(fd_client);
        
        char type = (char)*msg->type;
        printf("%c\n", type);
        switch(type){
            case MSG_REGISTRA_UTENTE:
                printf("Messaggio registrazione ricevuto\n");
                break;
            case MSG_FINE:
                
                break;
            //Aggiungere altri casi
            default:
                printf("Comando non riconosciuto!");
                //Invia messaggio comando sconosciuto?
                break;
        }

        
        free(msg->msg);
        free(msg->type);
    }


    /*while(1){
        int logged_in = 0;
        while(!logged_in){


            logged_in = 1;//sei riuscito a loggarti
        }
        int in_game = 1;
        //passi a gioco (sei loggato)        
        while(in_game) {
            

        }
    }*/
}

/*
void* asdrubale (void* arg) {
    //
    int fd_client = *(int*) arg;
    char type;
    printf("client:%d, connesso\n",fd_client);
    //Ricevo i messaggi dal client e li memorizzo in 'input'
    char* input = Ade(fd_client, &type);
    int prova = CercaUtente(list, input);
    printf("risultato prova1:%d\n", prova);
    //Controllo se il tipo del primo messaggio dal client è un comando di fine o una registrazione, in questo caso controllo se
    //il nome utente inviato come input è uguale a quello di un altro utente o meno. Entro nel while in uno di questi casi
    while ((type != MSG_REGISTRA_UTENTE && type != MSG_FINE) || prova == 0) {
        //Controllo subito se il nome utente è uguale a quello di un altro client oppure no
        if (prova == 0) 
            Caronte(fd_client, "Scrivi un altro nome utente: quello inviato è già stato preso\n", MSG_ERR);
        else{
            //Il primo comando inviato non è la registrazione dell'utente: invio il messaggio al client e gli permetto di riprovare a registrarsi
            Caronte(fd_client, "Errore: devi prima registrarti\n", MSG_ERR);
        }
        //Libero lo spazio di memoria collegato ad 'input'
        free(input);
        //Memorizzo in 'input' il nuovo messaggio inviato dal client
        input = Ade(fd_client, &type);
        prova = CercaUtente(list, input);
        printf("%s\n", input);
        printf("risultato prova2:%d\n",prova );
    }
    //È stato inviato il comando fine: termino il thread del client
    if (type == MSG_FINE) {
        pthread_exit(NULL);
    }
    //Aggiungo il giocatore alla lista
    //printf("%s\n",input);
    Aggiungi_Giocatore(&list, input, fd_client);
    printf("lista giocatori:%d\n",Numero_Giocatori(list));
    printf("fd:%d\n",list->fd_client);
    printf("nome:%s\n",list->nome);
    printf("punti:%d\n",list->punti);
    printf("thread:%lu\n",list->thread);
    Caronte(fd_client, "Nome utente valido", MSG_OK);
    free(input);
    //Controllo se il tipo del messaggio è fine o Cancella utente: fino a che non è nessuno dei due, il client gioca e può inviare 
    //tutti i comandi a sua disposizione
    while(type != MSG_FINE && type != MSG_CANCELLA_UTENTE) {
        input = Ade(fd_client, &type);
        //QUI DENTRO IL CLIENT GIOCA





    }
    //Se il tipo del messaggio è CANCELLA_UTENTE allora elimino l'utente dalla lista e chiudo il thread
    if (type == MSG_CANCELLA_UTENTE) {
        Rimuovi_Giocatore(&list, pthread_self());
        pthread_exit(NULL);
    }
    Rimuovi_Giocatore(&list, pthread_self());
    printf("Chiusura client\n");
    printf("lista giocatori:%d\n",Numero_Giocatori(list));
    return NULL;
}
*/

//Definisco la funzione che gestisce le fasi della partita
void* Argo(void* arg) {
    return NULL;
}


int main (int argc, char* argv[]) {
    // gestire errori per numero di parametri, ecc...

    //Creo una Matrice 4x4


    //Creo la lista vuota di Giocatori
    //Creo la lista di giocatori
    //Lista_Giocatori list = NULL;

    //creo l'identificatore per il socket, salvo e casto come intero la porta del server
    int fd_server, porta_server = atoi(argv[2]), retvalue;
    
    //Creo il thread che gestisce le fasi della partita
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

    return 0;
}

