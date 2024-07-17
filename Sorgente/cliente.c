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
#include "../Header/macro.h"
#include "../Header/Protocolli.h"
#include "../Header/Lista.h"
#include "../Header/Matrice.h"
#include "../Header/LogFun.h"

//Inizializzo variabili globali
Lettera** matrice;
int fd_server;
pthread_mutex_t messaggio_mutex;

//Definisco la funzione che gestise la SIGINT
void GestoreSigint(int signum) {
    int retvalue;
    Caronte(fd_server, "Chiusura Client tramite SIGINT", MSG_FINE);
    SYSC(retvalue, close(fd_server), "Errore nella close Client");
    exit(EXIT_SUCCESS);
}

//Funzione che riceve dal Server
void* receiver(void* args) {
    //Creo un variabile che memorizza il messaggio di ritorno dal server
    Msg* received_msg;

    while(1) {
        //Memorizzo il messaggio di ritorno dal server e il tipo
        received_msg = Ade(fd_server);
        char type = (char)*received_msg->type;
        //Switch sul tipo di ritorno
        switch(type){
            case MSG_OK:
                //Messaggio di Ok, stampo il messaggio ricevuto
                pthread_mutex_unlock(&messaggio_mutex);
                printf("\n%s\n", received_msg->msg);
                fflush(0);
                break;

            case MSG_ERR:
                //Messaggio di Errore, stampo il messaggio ricevuto
                pthread_mutex_unlock(&messaggio_mutex);
                printf("\n%s\n", received_msg->msg);
                fflush(0);
                break;

            case MSG_FINE:
                //Messaggio di Fine, dico al server di chiudere il fd e il collegamento
                pthread_mutex_unlock(&messaggio_mutex);
                Caronte(fd_server,"Chiusura client", MSG_FINE);
                exit(EXIT_SUCCESS);

            case MSG_MATRICE:
                //Viene allocata la matrice, caricata tramite il messaggio di ritorno dal server e stampata
                matrice = Crea_Matrix();
                Carica_Matrix_Stringa(matrice, received_msg->msg);
                printf("\n");
                fflush(0);
                Stampa_Matrix(matrice);
                
                break;

            case MSG_PUNTI_PAROLA:
                //Messaggio di ritorno dopo aver inserito una parola della matrice, stampo il punteggio ricevuto
                pthread_mutex_unlock(&messaggio_mutex);
                printf("\nhai totalizzato %s punti!\n", received_msg->msg);
                fflush(0);
                break;

            case MSG_TEMPO_PARTITA:
                //Messaggio di Tempo restante, stampo il messaggio ricevuto
                pthread_mutex_unlock(&messaggio_mutex);
                printf("\n%s rimanenti\n", received_msg->msg);
                fflush(0);
                break;

            case MSG_PUNTI_FINALI:
                //Messaggio di Classifica, stampo la classifica ricevuta
                pthread_mutex_unlock(&messaggio_mutex);
                printf("\nClassifica generale:\n");
                fflush(0);
                printf("%s\n", received_msg->msg);
                fflush(0);
                break;

            default:
                //Messaggio di default, stampo il messaggio ricevuto
                pthread_mutex_unlock(&messaggio_mutex);
                printf("\n%s\n", received_msg->msg);
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
        perror("Numero sbagliato di parametri passati!\n");
        exit(EXIT_SUCCESS);
    }

    //controllo se il secondo parametro è il nome del server...?
    if (strlen(argv[1]) < 9 || strlen(argv[1]) > 15) {
        perror("Errore nome server\n");
        exit(EXIT_SUCCESS);
    }

    //controllo se il terzo parametro è la porta del server (intero)
    if (atoi(argv[2]) == 0) {
        perror("Errore porta server\n");
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
        //Stampo il prompt per i comandi del paroliere
        printf("%s", "\n[PROMPT PAROLIERE]-->");
        fflush(0);
        //alloco la quantità di caratteri massimi possibili, contando anche la chat di gioco
        char* cmz = malloc(512*sizeof(char));
        //leggo il messaggio di input che l'utente scrive al client
        SYSC(nread, read(STDIN_FILENO,cmz, 134), "Errore Read");
        cmz[nread] = '\0';

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
        }
        //Messaggio per richiedere la matrice corrente (verrà inviato in concomitanza anche il tempo restante della partita o della pausa)
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
        //Messaggio per mostrare la bacheca dei messaggi
        if (strcmp(cmz,"show-msg\n") == 0){
            Caronte(fd_server,"Voglio vedere la bacheca\n",MSG_SHOW_BACHECA);
            free(cmz);
            continue;
        }
        //Tokenizzo per gestire il secondo input dopo il comando
        char* token;
        token = strtok(cmz, " ");

        ///Messaggio per registrare un giocatore
        if (strcmp(token, "registra_utente") == 0) {
            token = strtok(NULL, "\n");
            //Gestione errore token vuoto
            if (token == NULL) {
                printf("Nome utente non valido\n");
                fflush(0);
                pthread_mutex_unlock(&messaggio_mutex);
                continue;
            }
            //printf("return 0\n");         DEBUGG
            Caronte(fd_server, token, MSG_REGISTRA_UTENTE);
            free(cmz);
            continue;
        }
        //Messaggio per loggare un giocatore già registrato
        if(strcmp(token, "login_utente") == 0) {
            token = strtok(NULL, "\n");
            //Gestione errore token vuoto
            if (token == NULL) {
                printf("Nome utente non valido\n");
                fflush(0);
                pthread_mutex_unlock(&messaggio_mutex);
                continue;
            }
            Caronte(fd_server, token,MSG_LOGIN_UTENTE);
            free(cmz);
            continue;
        }
        //Messaggio per proporre una parola al server
        if (strcmp(token, "p") == 0) {
            token = strtok(NULL, "\n");
            //Gestione errore token vuoto
            if (token == NULL) {
                printf("Parola non valida\n");
                fflush(0);
                pthread_mutex_unlock(&messaggio_mutex);
                continue;
            }
            Caronte(fd_server, token, MSG_PAROLA);
            free(cmz);
            continue;
        }
        //Messaggio per scrivere sulla bacheca generale
        if (strcmp(token,"msg") == 0){
            token = strtok(NULL,"\n");
            //Gestione errore token vuoto
            if (token == NULL) {
                printf("Messaggio non valido\n");
                fflush(0);
                pthread_mutex_unlock(&messaggio_mutex);
                continue;
            }
            Caronte(fd_server,token,MSG_POST_BACHECA);
            free(cmz);
            continue;
        }
        //Qualsiasi altro tipo di comando non presente in quelli soprastanti
        printf("Comando non disponibile\n");
        fflush(0);
        pthread_mutex_unlock(&messaggio_mutex);
        free(cmz);

    }
    
}

