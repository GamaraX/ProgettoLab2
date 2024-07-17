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
#include <getopt.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "../Header/macro.h"
#include "../Header/Protocolli.h"
#include "../Header/Lista.h"
#include "../Header/Matrice.h"
#include "../Header/LogFun.h"
#include "../Header/Dizionario.h"
#include "../Header/Bacheca.h"
//Serve per non far crashare il programma quando un parametro non viene riconosciuto
extern int opterr;

typedef struct imp {
    Lettera** matrice; //Matrice corrente
    char* file_matrice; //File matrice usata
    int durata_minuti; //Durata partita
    int seed;   //Seed usato
    char* file_diz;  //File dizionario
    int tempo_disconnessione; //tempo di disconnessione oltre il quale disconnetto il client
}Impostazioni_Gioco;

//Enum utilizzata per la getopt
enum {
    OPT_MATRICE = 1,
    OPT_DURATA,
    OPT_SEED,
    OPT_DIZ,
    OPT_DISCONNECT,
};

char* tempo(int max_dur);

//Inizializzo variabili globali
time_t starttime;
char* filemat= NULL; //"../Eseguibili/matrici_disponibili.txt";
char* filediz = "../Eseguibili/dictionary_ita.txt";
Lettera** matrice;
Lista_Giocatori_Concorrente* lista;
int ingame = 0;
char* parole[279890];
int conteggio_parole;
Lista_FDCLIENT lista_fd;




//Variabili globali per produttore consumatore scorer
int conteggio_player;
Lista_Giocatori Lista_Scorer;
pthread_mutex_t scorer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t scorer_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t fd_mutex = PTHREAD_MUTEX_INITIALIZER;

//Definisco la funzione che gestisce SIGUSR1
void GestoreSigUsr1(int signum) {
    pthread_mutex_lock(&scorer_mutex);
    //printf("tid: %ld\n", pthread_self()); DEBUG
    //fflush(0);

    //Scorro la lista fino a che non arrivo alla fine
    Lista_Giocatori head_giocatori = lista->lista;
    while (head_giocatori != NULL) {
        if (head_giocatori->thread == pthread_self()) {

            //Copio la testa del giocatore
            Lista_Giocatori new_head = malloc(sizeof(Giocatore));
            //Controllo se c'è un errore di allocazione memoria
            if (new_head == NULL) {
                perror("Errore allocazione memoria");
                pthread_mutex_unlock(&scorer_mutex);
                return;
            }
            
            //Alloco un nuovo nodo e lo metto nella lista dello scorer
            new_head->fd_client = head_giocatori->fd_client;
            new_head->nome = strdup(head_giocatori->nome);
            if (new_head->nome == NULL) {
                perror("Errore allocazione memoria");
                free(new_head);
                pthread_mutex_unlock(&scorer_mutex);
                return;
            }
            //Pusho il nodo nella lista dello scorer così che il consumatore possa recuperarlo
            new_head->punti = head_giocatori->punti;
            new_head->thread = head_giocatori->thread;
            new_head->next = Lista_Scorer;
            Lista_Scorer = new_head;
            //Azzero i punteggi e la lista delle parole di ogni giocatore
            head_giocatori->punti = 0;
            head_giocatori->lista_parole = NULL;
            //Segnalo allo scorer che ho inserito in coda
            pthread_cond_signal(&scorer_cond);
            break;
        }
        head_giocatori = head_giocatori->next;
    }

    pthread_mutex_unlock(&scorer_mutex);
    return;    
}
//Definisco la funzione che gestise la SIGINT
void GestoreSigint(int signum) {
    pthread_mutex_lock(&fd_mutex);
    //Scorro la lista dei fd fino alla fine, invio ad ogni client il messaggio che il server si sta chiudendo
    Lista_FDCLIENT clietemp = lista_fd;
    while(clietemp != NULL) {
        Caronte(clietemp->fd_client, "Il server si sta chiudendo", MSG_FINE);
        clietemp = clietemp->next;
    }
    //Dealloco il dizionario
    Dealloca_Dizionario(parole, conteggio_parole);
    pthread_mutex_unlock(&fd_mutex);
    exit(EXIT_SUCCESS);
}
//Definisco Thread per la disconnessione
typedef struct timeout_thread{
    pthread_t pthread_id_father;
    int fd;
    int* last_command_timestamp;
    int required_inactivity;
}TimeoutThreadArgs;

//Thread che gestisce la disconnessione degli utenti per inattività
void* Bonker(void* args){
    //Recupero i parametri passati
    TimeoutThreadArgs* casted_args = (TimeoutThreadArgs*)args;
    while(1){
        //Aspetto per il tempo di disconnessione massimo
        sleep(casted_args->required_inactivity);
        //Controllo se l'utente ha superato il il tempo massimo di inattività
        if(time(NULL) - *casted_args->last_command_timestamp > casted_args->required_inactivity){           
            //Chiudo l'fd e controllo se è connesso: in tal caso devo sloggarlo
            pthread_mutex_lock(&lista->lock);
            Lista_Giocatori temp = lista->lista;
            while(temp != NULL){
                //Controllo se è loggato
                if(temp->fd_client == casted_args->fd && temp->loggato){
                    temp->loggato = 0;
                    break;
                }
                temp = temp->next;
            }
            pthread_mutex_unlock(&lista->lock);
            //Invio il messaggio di disconnessione per inattività
            Caronte(casted_args->fd, "Disconnessione per inattività", MSG_FINE);
            return NULL;
        }
    }
}

//Funzione di comparazione per il sorting  
int Confronta_Punti(const void *a, const void *b){

    Giocatore *giocatore1 = *(Giocatore **)a;
    Giocatore *giocatore2 = *(Giocatore **)b;
    return giocatore2->punti - giocatore1->punti;
}

//Thread Scorer
void *Thread_Scorer(void *args){

    int numgioc;
    //Metto nella lista dello scorer solo i giocatori loggati, quindi in partita
    numgioc = Numero_Giocatori_Loggati(lista);
    Lista_Giocatori giocatori_raccolti[numgioc];
    //Scorro la lista
    for (int i = 0; i < numgioc; i++){
        pthread_mutex_lock(&scorer_mutex);
        //Controllo se all'interno della lista dello scorer sono presenti dei giocatori, se non lo sono aspetto
        while (Lista_Scorer == NULL){
            pthread_cond_wait(&scorer_cond, &scorer_mutex);
        }
        //Inserisco i giocatori nella lista dei giocatori raccolti loggati
        Giocatore *temp = Lista_Scorer;
        Lista_Scorer = Lista_Scorer->next;
        giocatori_raccolti[i] = temp;
        pthread_mutex_unlock(&scorer_mutex);
    }
    //Qsort sui giocatori raccolti
    qsort(giocatori_raccolti, numgioc, sizeof(Giocatore *), Confronta_Punti);

    //Alloco lo spazio per la classifica e inserisco il giocatore e di seguito il suo punteggio
    char *classifica = malloc(1024);
    for (int i = 0; i < numgioc; i++){
        char *temp = malloc(128);
        sprintf(temp, "%s;%d\n", giocatori_raccolti[i]->nome, giocatori_raccolti[i]->punti);
        strcat(classifica, temp);
    }
    //Invio classifica come CSV a tutti i giocatori
    for (int i = 0; i < numgioc; i++){
        Caronte(giocatori_raccolti[i]->fd_client, classifica, MSG_PUNTI_FINALI);
    }
    //Scrivo il log della Classifica
    ScriviLog("Scorer", "Classifica\n", classifica);
    pthread_exit(NULL);
}

//Definisco la funzione che gestisce le fasi della partita
void* Fasi_Partita(void* arg) {
    Impostazioni_Gioco* argcast = (Impostazioni_Gioco*) arg;
    FILE* tempfd;

    //Conteggio le parole e carico il Dizionario in memoria 
    conteggio_parole = Carica_Dizionario(argcast->file_diz, parole);
    
    //Prendo e apro il file di matrice, se è presente
    if (argcast->file_matrice != NULL) {
        tempfd = fopen(argcast->file_matrice,"r");
    }

    while(1) {
        printf("Matrice caricata: %s\n", argcast->file_matrice);
        fflush(0);
        //Se è presente un file di matrice, carico la riga della matrice, altrimenti la genero casualmente
        if (argcast->file_matrice != NULL) {
            Carica_Matrix_File(tempfd, matrice);
        }
        else{
            Genera_Matrix(matrice, argcast->seed);
        }
        //Alloco lo spazio per inserire e concatenare in una stringa la matrice passata
        char* stringa_matrice = malloc(sizeof(char)*64);
        for (int i = 0; i < 4; i++) {
            for(int j = 0; j < 4; j++) {
                strcat(stringa_matrice, matrice[i][j].lettera);
                strcat(stringa_matrice, " "); 
            }
        }
        //Appena creata, invio la matrice a tutti i giocatori loggati
        pthread_mutex_lock(&lista->lock);
            Lista_Giocatori listatemp = lista->lista;
            while(listatemp != NULL){
                if (listatemp->loggato == 1) {
                    Caronte(listatemp->fd_client, stringa_matrice, MSG_MATRICE);
                    Caronte(listatemp->fd_client, tempo(argcast->durata_minuti*60), MSG_TEMPO_PARTITA);
                }
                listatemp = listatemp->next;
            }

        pthread_mutex_unlock(&lista->lock);
        
        //Stampa di debug
        Stampa_Matrix(matrice);
                                                        //La partita comincia
        ingame = 1;
        starttime = time(NULL);
        //sleep(20); Sleep di debug
        sleep(argcast->durata_minuti * 60);
                                                        //La partita è finita
        ingame = 0;
        starttime = time(NULL);
        //Controllo quanti player loggati ci sono quando 
        pthread_mutex_lock(&lista->lock);
        Giocatore* temp = lista->lista;
        pthread_mutex_unlock(&lista->lock);
        //Faccio partire uno scorer e n segnali tanti quanti sono i client loggati per attivare i produttori
        pthread_t scorer;
        pthread_create(&scorer, NULL, Thread_Scorer, NULL);
        while (temp != NULL) {
            if (temp->loggato == 1) {
                pthread_kill(temp->thread, SIGUSR1);
                Caronte(temp->fd_client, "Partita conclusa, inizio stato di pausa\n", MSG_OK);
            }
            temp = temp->next;
        }
        //sleep(5); Sleep di DEBUG
        sleep(60);
    }
    return NULL;
}

//Thread che gestisce ogni client
void* Client_Handler (void* arg) {

    //Struct sigaction
    struct sigaction sau;
    sau.sa_handler = GestoreSigUsr1;
    sau.sa_flags = SA_RESTART;
    sigemptyset(&sau.sa_mask);
    sau.sa_handler = GestoreSigUsr1;

    //Associo il GestoreSigUsr1 al segnale di SIGUSR1
    sigaction(SIGUSR1, &sau, NULL);
    
    //Recupero i dati del thread
    ThreadArgs* thread_args = (ThreadArgs*) arg;
    int fd_client = thread_args->fd_client;
    Lista_Giocatori_Concorrente* lista = thread_args->lista;
    int tempo_partita = thread_args->tempo_partita;

    //Aggiungo anche alla lista degli fd quando un un client si collega
    pthread_mutex_lock(&fd_mutex);
    Lista_FDCLIENT lista_fd_new = (Lista_FDCLIENT)malloc(sizeof(FDCLIENT));
    lista_fd_new->next = lista_fd;
    lista_fd_new->fd_client = thread_args->fd_client;
    //Aggiorno il puntatore alla testa di lista fd
    lista_fd = lista_fd_new;
    pthread_mutex_unlock(&fd_mutex);


    //Inizializzo variabili
    Giocatore* giocatore = NULL;
    Msg* msg;
    int punti;
    int messaggi_inseriti;
    Messaggio* messaggi;
    //Debugging
    printf("Connesso client su fd: %d\n", fd_client);

    //Imposto gli argomenti del thread che gestisce la disconnessione
    TimeoutThreadArgs* timeout_thread_args = (TimeoutThreadArgs*)malloc(sizeof(TimeoutThreadArgs));
    timeout_thread_args->fd = fd_client;
    timeout_thread_args->pthread_id_father = pthread_self();
    timeout_thread_args->required_inactivity = thread_args->tempo_disconnessione*60; 
    //timeout_thread_args->required_inactivity = 20; Timeout di DEBUG
    int last_command_timestamp = 0;
    timeout_thread_args->last_command_timestamp = &last_command_timestamp;

    //Faccio partire il Thread che gestisce la disconnessione
    pthread_t Disconnettore;
    pthread_create(&Disconnettore,NULL,Bonker, timeout_thread_args);

    while (1) {
        //Memorizzo il messaggtio inviato dal client
        msg = Ade(fd_client);
        //Memorizzo il tipo di messaggio inviato dal client
        char type = (char)*msg->type;
        //printf("%c\n",type);
        //fflush(0);
        //printf("%s\n", msg->msg);
        //fflush(0);

        //Alloco lo spazio per la matrice
        char* stringa_matrice = malloc(sizeof(char)*64);
        //Faccio uno switch su tutti i possibili tipi di messaggi che il client può inviare, e gestisco i vari casi speciali
        switch(type){
            case MSG_REGISTRA_UTENTE:
                //Controllo se il client è già loggato
                if(giocatore != NULL) {
                    Caronte(fd_client, "Errore sei già loggato", MSG_ERR);
                    break;
                }
                //Controllo se il nome è minore o uguale a 10 caratteri
                if (strlen(msg->msg) >= 11) {
                    Caronte(fd_client, "Errore nome utente troppo lungo", MSG_ERR);
                    break;
                }
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
                giocatore->lista_parole = NULL;
                Caronte(fd_client, "Registrazione effettuata correttamente", MSG_OK);
                //Invio la matrice al client appena si registra e invio anche il tempo partita
                for (int i = 0; i < 4; i++) {
                    for(int j = 0; j < 4; j++) {
                        strcat(stringa_matrice, matrice[i][j].lettera);
                        strcat(stringa_matrice, " "); //trovare modo elegante per fare questa cosa in una sola riga, soluzione temporanea per testare features
                    }
                }
                Caronte(fd_client, stringa_matrice, MSG_MATRICE);
                free(stringa_matrice);
                Caronte(fd_client, tempo(tempo_partita), MSG_TEMPO_PARTITA);
                //printf("%d\n",CercaUtente(lista, msg->msg));    Debugging
                break;
            case MSG_FINE:
                //printf("Client disconnesso\n");           Debugging
                if (giocatore != NULL) {
                    giocatore->loggato = 0;
                }
                lista_fd = Rimuovi_FD(lista_fd,fd_client);
                
                pthread_exit(NULL);
                break;
            case MSG_MATRICE:
                //Controllo se sono in fase di pausa o in game
                                            printf("%s\n", msg->msg);
                                            fflush(0);

                if (giocatore == NULL) {
                    Caronte(fd_client, "Errore, devi registrarti per visualizzare la matrice", MSG_ERR);
                    break;
                }
                
                if(ingame == 0) {
                    Caronte(fd_client, tempo(60), MSG_TEMPO_ATTESA);
                    break;
                }
                //Se non sono in fase di pausa, invio la matrice
                for (int i = 0; i < 4; i++) {
                    for(int j = 0; j < 4; j++) {
                        strcat(stringa_matrice, matrice[i][j].lettera);
                        strcat(stringa_matrice, " "); //trovare modo elegante per fare questa cosa in una sola riga, soluzione temporanea per testare features
                    }
                }
                Caronte(fd_client, stringa_matrice, MSG_MATRICE);
                free(stringa_matrice);
                Caronte(fd_client, tempo(tempo_partita), MSG_TEMPO_PARTITA);
                break;
            case MSG_PAROLA:
                if (giocatore == NULL) {
                    Caronte(fd_client, "Errore, non puoi inviare parole, non sei loggato", MSG_ERR);
                    break;
                }
                punti = 0;
                printf("%d\n", Controlla_Parola_Matrice(matrice, msg->msg));
                printf("%d\n",Ricerca_Binaria_Dizionario(parole ,conteggio_parole,msg->msg));
                if (Controlla_Parola_Matrice(matrice, msg->msg) == 1 && Ricerca_Binaria_Dizionario(parole ,conteggio_parole,msg->msg) == 1) {
                    if(strstr(msg->msg, "QU") != NULL || strstr(msg->msg, "qu") != NULL || strstr(msg->msg, "Qu") != NULL || strstr(msg->msg, "qU") != NULL) {
                        punti = strlen(msg->msg)-1;
                    }
                    else {
                        punti = strlen(msg->msg);
                    }
                    if (Cerca_Parola(giocatore->lista_parole, msg->msg) == 0) {
                        punti = 0;
                    }
                    if (punti != 0) {
                        Parola* par = (Parola*)malloc(sizeof(Parola));
                        par->parola = malloc(sizeof(char)*strlen(msg->msg)+1);
                        printf("ciao\n");
                        fflush(0);
                        strcpy(par->parola,msg->msg);
                        printf("%s", par->parola);
                        fflush(0);
                        par->next = giocatore->lista_parole;
                        giocatore->lista_parole = par; 
                    }

                    ScriviLog(giocatore->nome, "Immissione", msg->msg);
                    giocatore->punti += punti;
                    char spunti[3];
                    sprintf(spunti, "%d", punti);
                    Caronte(fd_client, spunti, MSG_PUNTI_PAROLA);
                    break;
                }
                Caronte(fd_client, "Parola non valida", MSG_ERR);
                break;
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
                //importante se, ci logghiamo l'fd deve cambiare 
                giocatore->fd_client = fd_client; //serve perché ad esempio lo scorer usa questo per inviare il msg classifica!
                Caronte(fd_client, "Utente loggato con successo!", MSG_OK);
                printf("login utente\n");
                fflush(0);
                break;
            case MSG_POST_BACHECA:
                if(giocatore == NULL) {
                    Caronte(fd_client, "Errore, non puoi inviare messaggi, non sei loggato", MSG_ERR);
                    break;
                }                
                inserisci_messaggio(msg->msg, giocatore->nome);
                Caronte(fd_client, "Messaggio inviato in bacheca!", MSG_OK);
                break;
            case MSG_SHOW_BACHECA:
                messaggi = leggi_messaggi(&messaggi_inseriti);
                for(int i=0; i<messaggi_inseriti;i++){
                    char* temp = malloc(256);
                    sprintf(temp, "%s: %s\n", messaggi[i].mittente, messaggi[i].messaggio);
                    Caronte(fd_client, temp, MSG_OK);
                    free(temp);
                }
                libera_messaggi(messaggi, messaggi_inseriti);
                break;
            //Aggiungere altri casi
            default:
                printf("Comando non riconosciuto!\n");
                Caronte(fd_client, "Errore Comando non disponibile", MSG_ERR);
                //Invia messaggio comando sconosciuto?
                fflush(0);
                break;
        }
        //qualcosa ha inviato se siamo qui, dunque updatiamo last_command_timestamp
        last_command_timestamp = time(NULL); //gli mettiamo il tempo corrente! 
        free(msg->msg);
        free(msg->type);
    }

}

int main (int argc, char* argv[]) {
    // gestire errori per numero di parametri, ecc...
    //controllo se il numero di parametri passati è giusto
    if (argc < 3) {
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
    //Prendo i parametri server e porta prima di fare la get opt
    char* nome_server = argv[1];
    int fd_server, porta_server = atoi(argv[2]), retvalue;


    int opt, indice_opt = 0, seed_ricevuto = 0, file_matrice_ricevuto = 0; 
    opterr = 0;

    struct option long_opt[] = {
        {"matrici", required_argument, 0, OPT_MATRICE},
        {"durata", required_argument, 0, OPT_DURATA},
        {"seed", required_argument, 0, OPT_SEED},
        {"diz", required_argument, 0, OPT_DIZ},
        {"disconnetti-dopo", required_argument, 0, OPT_DISCONNECT},
        {0, 0, 0, 0}
    };
    //definire qui valori default

    Impostazioni_Gioco* settings = malloc(sizeof(Impostazioni_Gioco));
    //Alloco una Matrice 4x4
    matrice = Crea_Matrix();
    //Settaggio delle impostazioni
    settings->matrice = matrice;
    settings->file_diz = filediz;
    settings->seed = rand();
    settings->durata_minuti = 3;
    settings->tempo_disconnessione = 5;
    
    //Gestire parametri passati opzionali
    while ((opt = getopt_long(argc, argv, "", long_opt, &indice_opt)) != -1) {
        switch (opt) {
            case OPT_MATRICE:
                FILE* matrice_open = fopen(optarg, "r");
                if(matrice_open == NULL){
                    printf("File matrice non esistente\n");
                    fflush(0);
                    exit(EXIT_SUCCESS);
                }
                file_matrice_ricevuto = 1;
                settings->file_matrice = strdup(optarg);
                Carica_Matrix_File(matrice_open, matrice);
                fclose(matrice_open);
                break;
            case OPT_DURATA:
                printf("Durata impostata: %s\n", optarg);
                settings->durata_minuti = atoi(optarg);
                break;
            case OPT_SEED:
                settings->seed = atoi(optarg);
                seed_ricevuto = 1;
                break;
            case OPT_DIZ:
                settings->file_diz = strdup(filediz);
                break;
            case OPT_DISCONNECT:
                settings->tempo_disconnessione = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [--matrici <value>] [--durata <value>] [--seed <value>] [--diz <value>] [--disconnetti-dopo <value>]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
        
    }
    
    if(seed_ricevuto+file_matrice_ricevuto == 2){
        printf("Impossibile inserire contemporaneamente sia il seed che il file matrice!");
        fflush(0);
        exit(EXIT_SUCCESS);    
    }else if(seed_ricevuto && !file_matrice_ricevuto){
        printf("Non è stato passato il file matrice, andremo con il seed...");
        fflush(0);
        settings->file_matrice = NULL;
    }

    //Struct sigaction
    struct sigaction sa;
    sa.sa_handler = GestoreSigint;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = GestoreSigint; //temporaneo

    //Associo il GestoreSigint al segnale di SIGINT
    sigaction(SIGINT, &sa, NULL);
    
    //Creo la lista vuota di Giocatori
    printf("Provo a creare la lista giocatori...\n");
    fflush(0);
    //Creo la lista di giocatori
    lista = malloc(sizeof(Lista_Giocatori_Concorrente));
    Inizializza_Lista(lista);
    InizializzaMutexLog();
    

    
    
    //Creo il thread che gestisce le fasi della partita
    pthread_t t_fasi_partita;

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

    //Creazione Thread che gestisce le fasi della partita
    SYST(retvalue, pthread_create(&t_fasi_partita, NULL, Fasi_Partita, settings), "Errore creazione t_fasi_partita");

    //Creo ed associo un thread per ogni client che si connette al server, ogni client applica la funzione Client_Handler
    while(1) {
        int fdtemp;
        pthread_t tidtemp;
        //Alloco spazio per i parametri del thread
        ThreadArgs* thread_args = malloc(sizeof(ThreadArgs));
        thread_args->file_diz = settings->file_diz;
        thread_args->tempo_partita = settings->durata_minuti*60;
        thread_args->tempo_disconnessione = settings->tempo_disconnessione;
        
                
        SYSC(fdtemp, accept(fd_server, NULL, 0), "Errore accept server");
        //Inizializza ThreadArgs
        thread_args->fd_client = fdtemp;
        thread_args->lista = lista;
        SYST(retvalue, pthread_create(&tidtemp, NULL, Client_Handler, thread_args), "Errore pthread create collegato client");
    }

    return 0;
}

//FUNZIONE CHE CALCOLA IL TEMPO RESTANTE
char* tempo(int max_dur){
    time_t end_time;
    //MEMORIZZA IL TEMPO ATTUALE
    time(&end_time);
    //CALCOLA LA DIFFERENZA TRA GLI ISTANTI DI TEMPO
    double elapsed = difftime(end_time, starttime);
    printf("%f\n", elapsed);
    fflush(0);
    //CALCOLA QUANTO TEMPO RIMANE
    double remaining = max_dur - elapsed; //
    char* mess = malloc(256);
    //PREPARA UN MESSAGGIO CON DENTRO I SECONDI RIMANENTI
    sprintf(mess,"%.0f secondi",remaining);
    //E LO RITORNA
    return mess;
}
