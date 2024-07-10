#include <pthread.h>

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
    struct gio* next;
}Giocatore;

//Lista Giocatori
typedef Giocatore * Lista_Giocatori;

//Definisco struttura lista con mutex
typedef struct {
    Lista_Giocatori lista;
    pthread_mutex_t lock;
} Lista_Giocatori_Concorrente;

typedef struct arg{
    int fd_client;
    pthread_t thread_id;
    Lista_Giocatori_Concorrente* lista;
} ThreadArgs;

//Funzione che permette al client di giocare
void* asdrubale (void* arg);
//Funzione per aggiungere Giocatori
void Aggiungi_Giocatore(Lista_Giocatori_Concorrente* lista_conc, char* nome, int fd);
//Funzione per rimuovere Giocatori
int Rimuovi_Giocatore(Lista_Giocatori_Concorrente* lista_conc, pthread_t tid);
//Funzione per contare il numero di Giocatori
int Numero_Giocatori(Lista_Giocatori_Concorrente* lista_conc);
//Funzione per cercare un nome utente
int CercaUtente(Lista_Giocatori_Concorrente* lista_conc, char* utente);
//Funzione che elimina la lista
void Elimina_Lista(Lista_Giocatori_Concorrente* lista_conc);
//Funzione che inizializza la lista a vuota
void Inizializza_Lista(Lista_Giocatori_Concorrente* lista_conc);
//Funzione che recupera il nome utente
Lista_Giocatori RecuperaUtente(Lista_Giocatori_Concorrente* lista_conc, char* utente);