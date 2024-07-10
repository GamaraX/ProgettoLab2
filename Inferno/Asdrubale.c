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
#include "../Purgatorio/LogFun.h"

void* asdrubale (void* arg) {
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
                //Lo aggiungo al file di Log
                ScriviLog(msg->msg, "Registrato", " ");
                //Adesso il giocatore è loggato
                giocatore = RecuperaUtente(lista,msg->msg);
                giocatore->loggato = 1;
                Caronte(fd_client, "Registrazione effettuata correttamente", MSG_OK);
                //printf("%d\n",CercaUtente(lista, msg->msg));    Debugging
                //Sistemo il buffer
                fflush(0);
                break;
            case MSG_FINE:
                //printf("Client disconnesso\n");           Debugging
                if (giocatore != NULL) {
                    giocatore->loggato = 0;
                }
                pthread_exit(NULL);
                break;
            case MSG_MATRICE:
                printf("Matrice");
                break;
            case MSG_CANCELLA_UTENTE:
                if (CercaUtente(lista, msg->msg) != 0) {
                    Caronte(fd_client, "Errore, non puoi cancellare un utente che non esiste", MSG_ERR);
                    break;
                }
                Rimuovi_Giocatore(lista,pthread_self());
                ScriviLog(msg->msg, "Cancellato", " ");
                Caronte(fd_client, "Cancellazione utente effettuata correttamente", MSG_OK);
                printf("Cancello utente");
                fflush(0);
                break;
            case MSG_LOGIN_UTENTE:
                Lista_Giocatori listatemp = RecuperaUtente(lista,msg->msg);
                printf(msg->msg);
                fflush(0);
                printf("Ricerca terminata");
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
                printf("login utente");
                fflush(0);
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