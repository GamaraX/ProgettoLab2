#include <pthread.h>

//
typedef struct par {
    char* parola;
    struct par* next;
} Parola;
//
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
//Funzione per aggiungere Giocatori
void Aggiungi_Giocatore(Lista_Giocatori* lista, char* nome, int fd);
//Funzione per rimuovere Giocatori
int Rimuovi_Giocatore(Lista_Giocatori lista, pthread_t tid);
//Funzione per contare il numero di Giocatori
int Numero_Giocatori(Lista_Giocatori lista);
//Funzione per cercare un nome utente
int CercaUtente(Lista_Giocatori lista, char* utente);
