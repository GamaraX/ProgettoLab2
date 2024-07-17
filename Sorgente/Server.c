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

typedef struct imp {
    Lettera** matrice; //Matrice corrente
    char* file_matrice; //File matrice usata
    int durata_minuti; //Durata partita
    int seed;   //Seed usato
    char* file_diz;  //File dizionario
    int tempo_disconnessione; //tempo di disconnessione oltre il quale disconnetto il client
}Impostazioni_Gioco;

enum {
    OPT_MATRICE = 1,
    OPT_DURATA,
    OPT_SEED,
    OPT_DIZ,
    OPT_DISCONNECT,
};

char* tempo(int max_dur);

//#todo controllare ogni tot secondi se currenttimestamp - timestampultimocomandoSINGOLOUTENTE chiudi connessione, fai un thread che prende in ingresso il timestamp e il giocatore come puntatori!!!!!!
//Inizializzo variabili globali
time_t starttime;
char* filemat= "../Eseguibili/matrici_disponibili.txt";
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
// Definisco la funzione che gestisce SIGUSR1
void GestoreSigUsr1(int signum) {
    pthread_mutex_lock(&scorer_mutex);
    printf("tid: %ld\n", pthread_self());
    fflush(0);

    Lista_Giocatori head_giocatori = lista->lista;
    while (head_giocatori != NULL) {
        if (head_giocatori->thread == pthread_self()) {

            // Copiando la testa del giocatore
            Lista_Giocatori new_head = malloc(sizeof(Giocatore));
            if (new_head == NULL) {
                perror("Errore allocazione memoria");
                pthread_mutex_unlock(&scorer_mutex);
                return;
            }

            new_head->fd_client = head_giocatori->fd_client;
            new_head->nome = strdup(head_giocatori->nome);
            if (new_head->nome == NULL) {
                perror("Errore allocazione memoria");
                free(new_head);
                pthread_mutex_unlock(&scorer_mutex);
                return;
            }
            new_head->punti = head_giocatori->punti;
            new_head->thread = head_giocatori->thread;
            new_head->next = Lista_Scorer;
            Lista_Scorer = new_head;
            head_giocatori->punti = 0;
            head_giocatori->lista_parole = NULL;
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
    Lista_FDCLIENT clietemp = lista_fd;
    while(clietemp != NULL) {
        Caronte(clietemp->fd_client, "Il server si sta chiudendo", MSG_FINE);
        clietemp = clietemp->next;
    }
    Dealloca_Dizionario(parole, conteggio_parole);
    pthread_mutex_unlock(&fd_mutex);
    exit(EXIT_SUCCESS);
}
//thread per la disconnessione
typedef struct timeout_thread{
    pthread_t pthread_id_father;
    int fd;
    int* last_command_timestamp;
    int required_inactivity;
}TimeoutThreadArgs;

void* The_Bonker(void* args){
    int retvalue;
    TimeoutThreadArgs* casted_args = (TimeoutThreadArgs*)args;
    while(1){
       sleep(casted_args->required_inactivity);
        if(time(NULL) - *casted_args->last_command_timestamp > casted_args->required_inactivity){
            //in questo caso l'utente non ha effettuato comandi per più del tempo richiesto per la disconnessione
            //quindi lo disconnetto
           
            //chiudo l'fd qui ma prima devo capire se per caso è connesso e sloggarlo (loggato=1)
            pthread_mutex_lock(&lista->lock);
            Lista_Giocatori temp = lista->lista;
            while(temp != NULL){
                if(temp->fd_client == casted_args->fd && temp->loggato){
                    //l'abbiamo trovato!
                    temp->loggato = 0;
                    break;
                }
                temp = temp->next;
            }
            pthread_mutex_unlock(&lista->lock);
            Caronte(casted_args->fd, "Disconnessione per inattività", MSG_FINE);
            //printf("%ld", casted_args->pthread_id_father);
            lista_fd = Rimuovi_FD(lista_fd, casted_args->fd);
            SYSC(retvalue, close(casted_args->fd), "Errore close Bonker");
            pthread_cancel(casted_args->pthread_id_father);
            return NULL;
        }
    }
}


int Confronta_Punti(const void *a, const void *b){

    Giocatore *giocatore1 = *(Giocatore **)a;
    Giocatore *giocatore2 = *(Giocatore **)b;
    return giocatore2->punti - giocatore1->punti;
}

// Thread Scorer
void *Thread_Scorer(void *args){
    int numgioc;

    numgioc = Numero_Giocatori_Loggati(lista);
    Lista_Giocatori giocatori_raccolti[numgioc];
    for (int i = 0; i < numgioc; i++){
        pthread_mutex_lock(&scorer_mutex);
        while (Lista_Scorer == NULL){
            pthread_cond_wait(&scorer_cond, &scorer_mutex);
        }
        Giocatore *temp = Lista_Scorer;
        Lista_Scorer = Lista_Scorer->next;
        giocatori_raccolti[i] = temp;
        pthread_mutex_unlock(&scorer_mutex);
    }
    // qsort giocatori raccolti
    qsort(giocatori_raccolti, numgioc, sizeof(Giocatore *), Confronta_Punti);

    char *classifica = malloc(1024);
    for (int i = 0; i < numgioc; i++){
        char *temp = malloc(128);
        sprintf(temp, "%s;%d\n", giocatori_raccolti[i]->nome, giocatori_raccolti[i]->punti);
        strcat(classifica, temp);
    }
    // invio classifica come csv a tutti i giocatori
    for (int i = 0; i < numgioc; i++){
        Caronte(giocatori_raccolti[i]->fd_client, classifica, MSG_PUNTI_FINALI);
    }
    ScriviLog("Scorer", "Classifica\n", classifica);
    pthread_exit(NULL);
}

//Definisco la funzione che gestisce le fasi della partita
void* Fasi_Partita(void* arg) {
    
    Impostazioni_Gioco* argcast = (Impostazioni_Gioco*) arg;
    argcast->file_matrice = filemat;
    FILE* tempfd;

    // CONTEGGIO PAROLE E CARICAMENTO DIZIONARIO (SE LO RITOCCHI IO TI STERILIZZO)
    conteggio_parole = Carica_Dizionario(argcast->file_diz, parole);
    
    //Prendo e apro il file
    if (argcast->file_matrice != NULL) {
        tempfd = fopen(argcast->file_matrice,"r");
    }

    while(1) {
        if (argcast->file_matrice != NULL) {
            Carica_Matrix_File(tempfd, matrice);
        }
        else{
            Genera_Matrix(matrice, argcast->seed);
        }
        char* stringa_matrice = malloc(sizeof(char)*64);
        for (int i = 0; i < 4; i++) {
            for(int j = 0; j < 4; j++) {
                strcat(stringa_matrice, matrice[i][j].lettera);
                strcat(stringa_matrice, " "); //trovare modo elegante per fare questa cosa in una sola riga, soluzione temporanea per testare features
            }
        }
        //Appena creata, invio la matrice a tutti i giocatori
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
        
            
        Stampa_Matrix(matrice);
        //La partita comincia
        ingame = 1;
        starttime = time(NULL);
        //sleep(20);
        sleep(argcast->durata_minuti * 60);
        
        //La partita è finita
        ingame = 0;
        starttime = time(NULL);
        //Controllo quanti player loggati ci sono quando 
        pthread_mutex_lock(&lista->lock);
        Giocatore* temp = lista->lista;
        pthread_mutex_unlock(&lista->lock);
        pthread_t scorer;
        pthread_create(&scorer, NULL, Thread_Scorer, NULL);
        while (temp != NULL) {
            if (temp->loggato == 1) {
                pthread_kill(temp->thread, SIGUSR1);
            }
            temp = temp->next;
        }
        //sleep(5);
        sleep(60);
        //Funzione che azzera i punti dei client
        //Funzione che azzera le parole dei client
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
    sau.sa_handler = GestoreSigUsr1; //temporaneo

    //Associo il GestoreSigUsr1 al segnale di SIGUSR1
    sigaction(SIGUSR1, &sau, NULL);

    //#todo variabile per timestamp ultimo comando  
    //#todo starta un thread e passa quel timestamp come puntatore, passa anche giocatore, e quando stacchi, ricordati di controllare se per caso si e` loggato, nel caso setta loggato=0
    
    //Recupero i dati del thread
    ThreadArgs* thread_args = (ThreadArgs*) arg;
    int fd_client = thread_args->fd_client;
    Lista_Giocatori_Concorrente* lista = thread_args->lista;
    int tempo_partita = thread_args->tempo_partita;
    Lista_FDCLIENT lista_fd_new = (Lista_FDCLIENT)malloc(sizeof(FDCLIENT)); //#FLAG
    lista_fd_new->next = lista_fd;
    lista_fd_new->fd_client = thread_args->fd_client;
    //ora aggiorno il puntatore alla testa di lista fd
    lista_fd = lista_fd_new;
    

    //Inizializzo variabili
    Giocatore* giocatore = NULL;
    Msg* msg;
    int punti;
    int messaggi_inseriti;
    Messaggio* messaggi;
    //Debugging
    printf("Connesso client su fd: %d\n", fd_client);



    //Facciamo partire il thread che controllerà se abbiamo inviato un msg da questo client negli ultimi tot secondi

    //preparo gli args
    TimeoutThreadArgs* timeout_thread_args = (TimeoutThreadArgs*)malloc(sizeof(TimeoutThreadArgs));
    timeout_thread_args->fd = fd_client;
    timeout_thread_args->pthread_id_father = pthread_self();
    //timeout_thread_args->required_inactivity = thread_args->tempo_disconnessione*60; 

    timeout_thread_args->required_inactivity = 20; //#FLAG
    int last_command_timestamp = 0;
    timeout_thread_args->last_command_timestamp = &last_command_timestamp;

    //qui invece lo faccio partire effettivamente
    pthread_t ciao;
    pthread_create(&ciao,NULL,The_Bonker, timeout_thread_args);

    while (1) {
        //Memorizzo il messaggtio inviato dal client
        msg = Ade(fd_client);


        //Memorizzo il tipo di messaggio inviato dal client
        char type = (char)*msg->type;
        printf("%c\n",type);
        fflush(0);
        printf("%s\n", msg->msg);
        fflush(0);
        char* stringa_matrice = malloc(sizeof(char)*64);
        
        //Faccio uno switch su tutti i possibili tipi di messaggi che il client può inviare, e gestisco i vari casi speciali
        switch(type){
            case MSG_REGISTRA_UTENTE:
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
                lista_fd = Rimuovi_FD (lista_fd,fd_client);
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

    int opt, indice_opt = 0, seed_ricevuto = 0; 

    struct option long_opt[] = {
        {"matrici", required_argument, 0, OPT_MATRICE},
        {"durata", required_argument, 0, OPT_DURATA},
        {"seed", required_argument, 0, OPT_SEED},
        {"diz", required_argument, 0, OPT_DIZ},
        {"disconnetti-dopo", required_argument, 0, OPT_DISCONNECT},
        {0, 0, 0, 0}
    };

    Impostazioni_Gioco* settings = malloc(sizeof(Impostazioni_Gioco));

    //Settaggio delle impostazioni
    settings->matrice = matrice;
    settings->file_diz = filediz;
    settings->seed = rand();
    settings->durata_minuti = 3;

    //Gestire parametri passati opzionali
    
    while ((opt = getopt_long(argc, argv, "", long_opt, &indice_opt)) != -1) {
        switch(opt) {
            case OPT_MATRICE:
                settings->file_matrice = optarg;
                break;
            case OPT_DURATA:
                settings->durata_minuti = atoi(optarg);
                break;
            case OPT_DIZ:
                settings->file_diz = optarg;
                break;
            case OPT_SEED:
                settings->seed = atoi(optarg);
                seed_ricevuto = 1;
                break;
            case OPT_DISCONNECT:
                settings->tempo_disconnessione = atoi(optarg);
                break;
            case '?':
                printf("Argomento aggiuntivo\n");
                break;
        }

        if (seed_ricevuto == 1 && strcmp(settings->file_matrice, filemat) != 0) {
            perror("Errore, hai inserito sia un seed che un file matrice\n");
            exit(EXIT_SUCCESS);
        }
        srand(settings->seed);
        return 0;
    }

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
    pthread_t t_fasi_partita;

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
