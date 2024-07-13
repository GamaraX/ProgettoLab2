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
#include "../Purgatorio/ListaClient.h"
#include "../Purgatorio/Matrice.h"
#include "../Purgatorio/LogFun.h"
#include "../Purgatorio/Dizionario.h"



//#todo controllare ogni tot secondi se currenttimestamp - timestampultimocomandoSINGOLOUTENTE chiudi connessione, fai un thread che prende in ingresso il timestamp e il giocatore come puntatori!!!!!!
//Inizializzo variabili globali
Lettera** matrice;
Lista_Giocatori_Concorrente* lista;
int ingame = 0;


//Definisco la funzione che gestise la SIGINT
void GestoreSigint(int signum) {
    Lista_Giocatori gioctemp = lista->lista;
    while(gioctemp != NULL) {
        if(gioctemp->loggato) {
            Caronte(gioctemp->fd_client, "Il server si sta chiudendo", MSG_FINE);
        }
        gioctemp = gioctemp->next;
    }
    exit(EXIT_SUCCESS);
}

//Definisco la funzione che gestisce le fasi della partita
void* Argo(void* arg) {
    int offset = 0;
    return NULL;
}

void* asdrubale (void* arg) {

    //
    int retvalue;
    //#todo variabile per timestamp ultimo comando  
    //#todo starta un thread e passa quel timestamp come puntatore, passa anche giocatore, e quando stacchi, ricordati di controllare se per caso si e` loggato, nel caso setta loggato=0
    
    //Recupero i dati del thread
    ThreadArgs* thread_args = (ThreadArgs*) arg;
    int fd_client = thread_args->fd_client;
    pthread_t thread_id = thread_args->thread_id;
    Lista_Giocatori_Concorrente* lista = thread_args->lista;
    //Inizializzo variabili
    Giocatore* giocatore;
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
                if(giocatore != NULL) {
                    Caronte(fd_client, "Errore sei già loggato", MSG_ERR);
                    break;
                }
                //Controllo se il nome è minore o uguale a 10 caratteri
                if (strlen(msg->msg) > 11) 
                    Caronte(fd_client, "Errore nome utente troppo lungo", MSG_ERR);
                //Variabile per controllare se supera il prossimo controllo di soli caratteri alfanumerici
                int valido = 1;
                //Controllo se il messaggio contiene solo caratteri alfanumerici
                for (int i = 0; i < strlen(msg->msg); i++) {
                    printf("%c\n", msg->msg[i]);
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
                //Lo aggiungo al file di Log
                ScriviLog(msg->msg, "Registrato", " ");
                //Adesso il giocatore è loggato
                giocatore = RecuperaUtente(lista,msg->msg);
                giocatore->loggato = 1;
                Caronte(fd_client, "Registrazione effettuata correttamente", MSG_OK);
                //printf("%d\n",CercaUtente(lista, msg->msg));    Debugging
                break;
            case MSG_FINE:
                //printf("Client disconnesso\n");           Debugging
                if (giocatore != NULL) {
                    giocatore->loggato = 0;
                }
                pthread_exit(NULL);
                break;
            case MSG_MATRICE:
                //#todolater fare in modo che non venga ricaricata da file ogni volta ma cambiata da qualche handler che gestisce sto game
                //Controllo se sono in fase di pausa o in game
                printf("%s\n", msg->msg);
                fflush(0);
                //TEMPORANEO
                char* file = "../Paradiso/matrici_disponibili.txt";
                //int offset;
                //int* offsetp = &offset;
                //*offsetp = 0;
                                    //printf("ciao\n");
                                    //fflush(0);
                Carica_Matrix_File(file, matrice, 0);
                // TEMPORANEO

                //Se non sono in fase di pausa, invio la matrice
                char* stringa_matrice[64];
                for (int i = 0; i < 4; i++) {
                    for(int j = 0; j < 4; j++) {
                        strcat(stringa_matrice, matrice[i][j].lettera);
                        strcat(stringa_matrice, " "); //trovare modo elegante per fare questa cosa in una sola riga, soluzione temporanea per testare features
                    }
                }
                Caronte(fd_client, stringa_matrice, MSG_MATRICE);
                //TEMPORANEO POI DEVO INVIARE ANCHE IL TEMPO

                //Caronte(fd_client, "Questo è il tempo restante", MSG_TEMPO_PARTITA);
                break;
            case MSG_PAROLA:

            case MSG_CANCELLA_UTENTE:
                if(giocatore == NULL) {
                    Caronte(fd_client, "Errore, non sei loggato", MSG_ERR);
                    break;
                }
                char* tmpusername = Rimuovi_Giocatore(lista,giocatore->nome);
                printf("%s\n",tmpusername);
                fflush(0);
                if (strcmp("", tmpusername) == 0) {
                    Caronte(fd_client, "Errore, non ti sei registrato, quindi non puoi cancellarti", MSG_ERR);
                    break;
                }
                ScriviLog(tmpusername, "Cancellato", " ");
                Caronte(fd_client, "Cancellazione utente effettuata correttamente\n", MSG_OK);
                giocatore = NULL;
                printf("Cancello utente\n");
                fflush(0);
                break;
            case MSG_LOGIN_UTENTE:
                if(giocatore != NULL) {
                    Caronte(fd_client, "Errore già loggato", MSG_ERR);
                    break;
                }
                Lista_Giocatori listatemp = RecuperaUtente(lista,msg->msg);
                //printf("%s\n",msg->msg);      DEBUG
                fflush(0);
                printf("Ricerca terminata\n");
                fflush(0);
                if (listatemp == NULL) {
                    Caronte(fd_client, "Errore, il giocatore non si è mai registrato. Fare una nuova registrazione utente", MSG_ERR);
                    break;
                }
                if (listatemp->loggato) {
                    Caronte(fd_client, "Errore, un giocatore è già loggato con questo nome utente. Fare una nuova registrazione utente", MSG_ERR);
                    break;
                }
                giocatore = listatemp;
                giocatore->loggato = 1;
                Caronte(fd_client, "Utente loggato con successo!", MSG_OK);
                printf("login utente\n");
                fflush(0);
                break;
            case MSG_POST_BACHECA:
                
                break;
            case MSG_SHOW_BACHECA:

                break;
            //Aggiungere altri casi
            default:
                printf("Comando non riconosciuto!\n");
                //Invia messaggio comando sconosciuto?
                fflush(0);
                break;
        }

        free(msg->msg);
        free(msg->type);
    }

}

int main (int argc, char* argv[]) {
    // gestire errori per numero di parametri, ecc...

    //Struct sigaction
    struct sigaction sa;
    sa.sa_handler = GestoreSigint;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = GestoreSigint; //temporaneo

    //Associo il GestoreSigint al segnale di SIGINT
    sigaction(SIGINT, &sa, NULL);

    //Alloco una Matrice 4x4
    matrice = Crea_Matrix();
    char* file = "../Paradiso/matrici_disponibili.txt";
    char* file2 = "../Paradiso/dictionary_ita.txt";
    Carica_Matrix_File(file, matrice, 0);
    //Genera_Matrix(matrice, 2);
                    //printf("ciao\n");
                    //fflush(0);
    Stampa_Matrix(matrice);
    int contr;
    contr = Controlla_Parola_Matrice(matrice,"CASI");
    printf("%d\n", contr);
    fflush(0);
    int diz;
    //diz = Ricerca_Binaria_Dizionario(file2, "casi");
    //printf("%d\n", diz);
    //fflush(0);

    //Creo la lista vuota di Giocatori
    printf("Provo a creare la lista...\n");
    fflush(0);
    //Creo la lista di giocatori
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

