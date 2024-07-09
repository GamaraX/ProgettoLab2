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
//#include "../Purgatorio/LogFun.h"

//Temporaneo
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Inizializzo variabili globali
pthread_mutex_t log_mutex;
char* filelog;

//Funzione che inizializza i log
void InizializzaMutexLog() {
    pthread_mutex_init(&log_mutex, NULL);
}

//Funzione che cerca nel file di log
char* CercaLog(char* azione) {
    pthread_mutex_lock(&log_mutex);
    char* ris_azione = NULL;
    char* linea_file = NULL;
    char* token_azione;
    size_t len = 0;
    ssize_t cmplettura;
    //Apro il file in sola lettura
    FILE* tempfd = fopen("../Paradiso/Log.txt", "r");
    //Controllo se il file esiste o ci sono errori/corruzioni
    if (tempfd == NULL) {
        perror("Error opening file");
        pthread_mutex_unlock(&log_mutex);
        return NULL;
    }
    //Fino a che non arrivo alla fine del file, leggo il secondo parametro (che è l'azione eseguita dal client) e controllo se è l'azione che sto cercando
    while ((cmplettura = getline(&linea_file, &len, tempfd)) != -1) {
        token_azione = strtok(linea_file, ";");
        token_azione = strtok(NULL, ";");
        if (token_azione != NULL && strcmp(token_azione, azione) == 0) {
            ris_azione = strdup(azione); 
            break;
        }
    }
    free(linea_file);
    fclose(tempfd);
    pthread_mutex_unlock(&log_mutex);
    return ris_azione;
}

//Funzione che scrive nel file di log
void ScriviLog(char* utente, char* azione, char* testo) {
    //printf("%s %s %s\n", utente, azione, testo);              Debugging
    pthread_mutex_lock(&log_mutex);
    //Apro il file in append
    FILE* tempfd = fopen("../Paradiso/Log.txt", "a");
    //Controllo se il file esiste o ci sono errori/corruzioni
    if (tempfd == NULL) {
        perror("Error opening file");
        pthread_mutex_unlock(&log_mutex);
        return;
    }
    //Scrivo nel log la terna in formato nome utente;azione eseguita;testo (viene memorizzata la parola, altrimenti è un white space)
    fprintf(tempfd, "%s;%s;%s\n", utente, azione, testo);
    //Chiudo il file
    fclose(tempfd);
    pthread_mutex_unlock(&log_mutex);
}

//Definizione funzioni

void* asdrubale (void* arg) {
    //Recupero i dati del thread
    ThreadArgs* thread_args = (ThreadArgs*) arg;
    int fd_client = thread_args->fd_client;
    pthread_t thread_id = thread_args->thread_id;
    Lista_Giocatori_Concorrente* lista = thread_args->lista;
    //Inizializzo variabili
    Giocatore giocatore;
    Msg* msg;
    //Debugging
    printf("Connesso client su fd: %d\n", fd_client);

    while (1) {
        //Memorizzo il messaggio inviato dal client
        msg = Ade(fd_client);
        //Memorizzo il tipo di messaggio inviato dal client
        char type = (char)*msg->type;

        //Faccio uno switch su tutti i possibili tipi di messaggi che il client può inviare, e gestisco i vari casi speciali
        switch(type){
            case MSG_REGISTRA_UTENTE:
                //Controllo se il nome è minore o uguale a 10 caratteri
                if (strlen(msg->msg) > 11) 
                    Caronte(fd_client, "Errore nome utente troppo lungo", MSG_ERR);
                //Variabile per controllare se supera il prossimo controllo di soli caratteri alfanumerici
                int valido = 1;
                //Controllo se il messaggio contiene solo caratteri alfanumerici
                for (int i = 0; i < strlen(msg->msg); i++) {
                    printf("%c", msg->msg[i]);
                    fflush(0);
                    if(isdigit(msg->msg[i]) == 0 && isalpha(msg->msg[i]) == 0) {
                        Caronte(fd_client, "Errore carattere speciale immesso", MSG_ERR);
                        valido = 0;
                        break;
                    }
                }
                //Controllo se è stato inviato un nome utente valido
                if (!valido)
                    break;
                //Controllo se il nome utente è già stato preso
                if (CercaUtente(lista, msg->msg) == 0) {
                    Caronte(fd_client, "Errore nome utente già preso", MSG_ERR);
                    break;
                }
                //Dopo aver fatto tutti i controlli, aggiungo il client alla lista e invio il messaggio di OK
                Aggiungi_Giocatore(lista, msg->msg, fd_client);
                ScriviLog(msg->msg, "Registrato", " ");
                Caronte(fd_client, "Registrazione effettuata correttamente", MSG_OK);
                //printf("%d\n",CercaUtente(lista, msg->msg));    Debugging
                //Sistemo il buffer
                fflush(0);
                break;
            case MSG_FINE:
                //printf("Client disconnesso\n");           Debugging
                pthread_exit(NULL);
                fflush(0);
                break;
            case MSG_MATRICE:
                printf("Matrice");
                break;
            case MSG_CANCELLA_UTENTE:
                printf("Cancello utente");
                break;
            case MSG_LOGIN_UTENTE:
                printf("login utente");
                break;
            case MSG_POST_BACHECA:
                
                break;
            case MSG_SHOW_BACHECA:

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

}



//Definisco la funzione che gestisce le fasi della partita
void* Argo(void* arg) {
    return NULL;
}



int main (int argc, char* argv[]) {
    // gestire errori per numero di parametri, ecc...

    //Creo una Matrice 4x4


    //Creo la lista vuota di Giocatori
    printf("Provo a creare la lista...\n");
    //Creo la lista di giocatori
    Lista_Giocatori_Concorrente* lista;
    lista = malloc(sizeof(Lista_Giocatori_Concorrente));
    Inizializza_Lista(lista);
    InizializzaMutexLog();


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
        //Alloco spazio per i parametri del thread
        ThreadArgs* thread_args = malloc(sizeof(ThreadArgs));
                
        SYSC(fdtemp, accept(fd_server, NULL, 0), "Errore accept server");
        //Inizializza ThreadArgs
        thread_args->fd_client = fdtemp;
        thread_args->thread_id = tidtemp;
        thread_args->lista = lista;
        SYST(retvalue, pthread_create(&tidtemp, NULL, asdrubale, thread_args), "Errore pthread create collegato client");
    }

    return 0;
}

