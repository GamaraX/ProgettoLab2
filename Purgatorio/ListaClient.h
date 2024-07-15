#include <pthread.h>

//Creazione nodo Parola
typedef struct par {
    char* parola;
    int punteggio;
    struct par* next;
} Parola;

//Lista Parole
typedef Parola * Lista_Parole;
//#todo calcola punteggio di questa struct, poi calcola punteggio utente ecc.......ricordarsi di fare sempre le deallocazioni, funzione per aggiungere una parola indovinata blabla

//Creazione nodo Giocatore
typedef struct gio {
    pthread_t thread;
    char* nome;
    int punti;
    int fd_client;
    int loggato;
    //#todo ListaParole è una lista linkata che oltre al next (per scorrerla) ha due campi valore, uno char* per la parole e uno int per il punteggio guadagnato da quella parola
    Lista_Parole* lista_parole;
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
    pthread_t thread_id;
    Lista_Giocatori_Concorrente* lista;
    char* file_diz;
    int tempo_partita;
} ThreadArgs;

//Funzione che permette al client di giocare
void* asdrubale (void* arg);
//Funzione per aggiungere Giocatori
void Aggiungi_Giocatore(Lista_Giocatori_Concorrente* lista_conc, char* nome, int fd);
//Funzione per rimuovere Giocatori
char* Rimuovi_Giocatore(Lista_Giocatori_Concorrente* lista_conc, char* nome_utente);
//Funzione per contare il numero di Giocatori
int Numero_Giocatori(Lista_Giocatori_Concorrente* lista_conc);
//Funzione per cercare un giocatore con un determinato nome utente
int CercaUtente(Lista_Giocatori_Concorrente* lista_conc, char* utente);
//Funzione che elimina la lista
void Elimina_Lista(Lista_Giocatori_Concorrente* lista_conc);
//Funzione che inizializza la lista a vuota
void Inizializza_Lista(Lista_Giocatori_Concorrente* lista_conc);
//Funzione che recupera il nome utente di un giocatore
Lista_Giocatori RecuperaUtente(Lista_Giocatori_Concorrente* lista_conc, char* utente);