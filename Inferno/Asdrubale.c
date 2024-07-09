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
    Giocatore giocatore;
    Msg* msg;

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
