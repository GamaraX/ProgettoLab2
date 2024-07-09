#include <pthread.h>

//Creazione nodo Parola
typedef struct par {
    char* parola;
    struct par* next;
} Parola;

//Creazione nodo Giocatore
typedef struct gio {
    pthread_t thread;
    char* nome;
    int punti;
    int fd_client;
    struct gio* next;
}Giocatore;

//Lista Parole
typedef Parola * Lista_Parole;
//Lista Giocatori
typedef Giocatore * Lista_Giocatori;

//Definisco struttura lista con mutex
typedef struct {
    Lista_Giocatori lista;
    pthread_mutex_t lock;
} Lista_Giocatori_Concorrente;

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