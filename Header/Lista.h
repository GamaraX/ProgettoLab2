#include <pthread.h>

//Creazione nodo fd_client
typedef struct fd {
    int fd_client;
    struct fd* next;
}FDCLIENT;

//Lista fd_client
typedef FDCLIENT * Lista_FDCLIENT;

//Creazione nodo Parola
typedef struct par {
    char* parola;
    struct par* next;
} Parola;

//Lista Parole
typedef Parola * Lista_Parole;

//Creazione nodo Giocatore
typedef struct gio {
    pthread_t thread;
    char* nome;
    int punti;
    int fd_client;
    int loggato;
    Lista_Parole lista_parole;
    struct gio* next;
}Giocatore;

//Lista Giocatori
typedef Giocatore * Lista_Giocatori;

//Definisco struttura lista con mutex
typedef struct {
    Lista_Giocatori lista;
    pthread_mutex_t lock;
} Lista_Giocatori_Concorrente;

//Definisco struttura argomenti da passare al thread
typedef struct arg{
    int fd_client;
    Lista_Giocatori_Concorrente* lista;
    char* file_diz;
    int tempo_partita;
    int tempo_disconnessione;
} ThreadArgs;

//Funzione che permette al client di giocare
void* asdrubale (void* arg);
//Funzione per aggiungere Giocatori
void Aggiungi_Giocatore(Lista_Giocatori_Concorrente* lista_conc, char* nome, int fd);
//Funzione per rimuovere Giocatori
char* Rimuovi_Giocatore(Lista_Giocatori_Concorrente* lista_conc, char* nome_utente);
//Funzione per rimuovere i client tramite il fd
Lista_FDCLIENT Rimuovi_FD (Lista_FDCLIENT lista, int fd_client);
//Funzione per contare il numero di Giocatori
int Numero_Giocatori_Loggati(Lista_Giocatori_Concorrente* lista_conc);
//Funzione per cercare un giocatore con un determinato nome utente
int CercaUtente(Lista_Giocatori_Concorrente* lista_conc, char* utente);
//Funzione che elimina la lista
void Elimina_Lista(Lista_Giocatori_Concorrente* lista_conc);
//Funzione che inizializza la lista a vuota
void Inizializza_Lista(Lista_Giocatori_Concorrente* lista_conc);
//Funzione che recupera il nome utente di un giocatore
Lista_Giocatori RecuperaUtente(Lista_Giocatori_Concorrente* lista_conc, char* utente);
//Funzione che cerca le parole nella lista delle parole dell'utente
int Cerca_Parola(Lista_Parole lista_parole, char* parola_proposta);