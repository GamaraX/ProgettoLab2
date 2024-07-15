#define _XOPEN_SOURCE 700

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
#include "../Purgatorio/Lista.h"
#include "../Purgatorio/Matrice.h"
#include "../Purgatorio/LogFun.h"

//Inizializzo variabile globale
Lettera** matrice;
int fd_server;
pthread_mutex_t messaggio_mutex;

//Definisco la funzione che gestise la SIGINT
void GestoreSigint(int signum) {
    int retvalue;
    Caronte(fd_server, "Chiusura Client tramite SIGINT", MSG_FINE);
    SYSC(retvalue, close(fd_server), "Errore nella close Client");
}


void receiver(void* args) {
    //Creo un variabile che memorizza il messaggio di ritorno dal server
    Msg* received_msg;

    while(1) {
        //Memorizzo il messaggio di ritorno dal server e il tipo
        received_msg = Ade(fd_server);
        char type = (char)*received_msg->type;
        //Switch sul tipo di ritorno
        switch(type){
            case MSG_OK:
                pthread_mutex_unlock(&messaggio_mutex);
                printf("%s\n", received_msg->msg);
                fflush(0);
                break;
            case MSG_ERR:
                pthread_mutex_unlock(&messaggio_mutex);
                printf("%s\n", received_msg->msg);
                fflush(0);
                break;

            case MSG_FINE:
                pthread_mutex_unlock(&messaggio_mutex);
                exit(EXIT_SUCCESS);

            case MSG_MATRICE:
                //Viene allocata la matrice, caricata tramite il messaggio di ritorno dal server e stampata
                matrice = Crea_Matrix();
                Carica_Matrix_Stringa(matrice, received_msg->msg);
                printf("\n");
                fflush(0);
                Stampa_Matrix(matrice);
                //fare free matrice
                break;
            case MSG_PUNTI_PAROLA:
                pthread_mutex_unlock(&messaggio_mutex);
                printf("hai totalizzato %s punti!", received_msg->msg);
                fflush(0);
                break;
            case MSG_TEMPO_PARTITA:
                pthread_mutex_unlock(&messaggio_mutex);
                printf("%s rimanenti\n", received_msg->msg);
                fflush(0);
                break;
            default:
                pthread_mutex_unlock(&messaggio_mutex);
                printf("%s\n", received_msg->msg);
                fflush(0);
                break;
        }

    }
}



int main(int argc, char* argv[]) {
    //                   int retvalue;
    
    //Mutex per la mutua esclusione dei messaggi
    pthread_mutex_init(&messaggio_mutex, NULL);
    
    //controllo se il numero di parametri passati è giusto
    if (argc != 3) {
        perror("Numero sbagliato di parametri passati!");
        exit(EXIT_SUCCESS);
    }

    //controllo se il secondo parametro è il nome del server...?
    if (strlen(argv[1]) < 9 || strlen(argv[1]) > 15) {
        perror("Errore nome server");
        exit(EXIT_SUCCESS);
    }

    //controllo se il terzo parametro è la porta del server (intero)
    if (atoi(argv[2]) == 0) {
        perror("Errore porta server");
        exit(EXIT_SUCCESS);
    }

    //Struct sigaction
    struct sigaction sa;
    sa.sa_handler = GestoreSigint;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    //Associo il GestoreSigint al segnale di SIGINT
    sigaction(SIGINT, &sa, NULL);



    //creo l'identificatore per il socket, salvo e casto come intero la porta del server 
    int porta_server = atoi(argv[2]);

    //salvo il nome del server
    char* nome_server = argv[1];

    //creo una struct con le informazioni del server  
    struct sockaddr_in info_server;
    info_server.sin_family = AF_INET; 
    info_server.sin_port = htons(porta_server);
    info_server.sin_addr.s_addr = inet_addr(nome_server);

    //creo la socket del server e alloco il file descriptor del server 
    SYSC(fd_server, socket(AF_INET, SOCK_STREAM, 0),"Errore Socket");

    //ciclo e cerco di connettermi al server; se non riesco a connettermi perchè il server non è pronto, aspetto e riprovo a connettermi.
    //altrimenti esco e chiudo perchè c'è un altro tipo di errore
    while (connect(fd_server, (struct sockaddr *)&info_server, sizeof(info_server))) {
        if (errno == ENOENT)
            sleep(1);
        else { 
            perror("Errore nella connect");
            exit(errno);
        }
    }

    // Facciamo partire il thread receiver
    pthread_t receiver_thread;
    pthread_create(&receiver_thread, NULL, receiver, NULL);


    //ricevo i messaggi che l'utente invia come input al client, che poi comunicherà al server
    while(1) {
        ssize_t nread;
        pthread_mutex_lock(&messaggio_mutex);

        //alloco la quantità di caratteri massimi possibili, contando anche la chat di gioco
        char* cmz = malloc(134*sizeof(char));

        //leggo il messaggio di input che l'utente scrive al client
        printf("%s", "[PROMPT PAROLIERE]-->");
        fflush(0);

        SYSC(nread, read(STDIN_FILENO,cmz, 134), "Errore Read");
        //Messaggio di aiuto
        if(strcmp(cmz, "aiuto\n") == 0) {
            printf(HELP_MESSAGE);
            fflush(0);
            free(cmz);
            pthread_mutex_unlock(&messaggio_mutex);
            continue;
        }
        //Messaggio per terminare la comunicazione
        if (strcmp(cmz, "fine\n") == 0) {
            Caronte(fd_server, "Chiusura client", MSG_FINE);
            close(fd_server);
            free(cmz);
            exit(EXIT_SUCCESS);
            return 0;
        }
        //Messaggio per richiedere la matrice corrente
        if (strcmp(cmz, "matrice\n") == 0) {
            Caronte(fd_server, "Invio Matrice gioco corrente", MSG_MATRICE);
            free(cmz);
            continue;
        }
        //Messaggio per cancellare un giocatore
        if (strcmp(cmz, "cancella_registrazione\n") == 0) {
            Caronte(fd_server, "Cancello utente", MSG_CANCELLA_UTENTE);
            free(cmz);
            continue;
        }
        //Tokenizzo per gestire il secondo input dopo il comando
        char* token;
        token = strtok(cmz, " ");

        ///Messaggio per registrare un giocatore
        if (strcmp(token, "registra_utente") == 0) {
            token = strtok(NULL, "\n");
            //printf("return 0\n");         DEBUGG
            Caronte(fd_server, token,MSG_REGISTRA_UTENTE);
            free(cmz);
            continue;
        }
        //Messaggio per loggare un giocatore già registrato
        if(strcmp(token, "login_utente") == 0) {
            token = strtok(NULL, "\n");
            Caronte(fd_server, token,MSG_LOGIN_UTENTE);
            free(cmz);
            continue;
        }
        //Messaggio per proporre una parola al server
        if (strcmp(token, "p") == 0) {
            token = strtok(NULL, "\n");
            for (int i = 0; token[i] != '\n'; i++) {
                token[i] = toupper(token[i]);
            }
            Caronte(fd_server, token, MSG_PAROLA);
            free(cmz);
            continue;
        }
        //Qualsiasi altro tipo di comando non presente in quelli soprastanti
         Caronte(fd_server, "Comando non disponibile", MSG_ERR);
         fflush(0);
         free(cmz);

    }

}

