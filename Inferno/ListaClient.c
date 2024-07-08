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

//Definisco funzione che aggiunge giocatori
void Aggiungi_Giocatore(Lista_Giocatori* lista, char* nome, int fd) {
    //Creo un Giocatore nella lista dei Giocatori
    Giocatore* i = (Giocatore*) malloc(sizeof(Giocatore));
    //Associo il thread
    i->thread = pthread_self();
    //Associo il nome
    i->nome = (char*) malloc (strlen(nome)+1); 
    //Assegno i punti (che sono 0 all'inizio della partita)
    i->punti = 0;
    //Associo il fd
    i->fd_client = fd;
    //Faccio puntare alla testa della lista
    i->next = *lista;
    *lista = i;
    return;
}

int Rimuovi_Giocatore(Lista_Giocatori* lista, pthread_t tid) {
    //Caso base
    if (lista == NULL)
        return 1;
    //Controllo se il tid coincide
    if ((*lista)->thread == tid) {
        printf("Giocatore eliminato\n");
        Giocatore* temp = *lista;
        *lista = (*lista)->next;
        free(temp);
        return 0;
    }
    //Cerco il prossimo tid nella lista
    if ((*lista)->next->thread == tid) {
        free((*lista)->next->nome);
        Lista_Giocatori temp = (*lista)->next->next;
        free((*lista)->next);
        (*lista)->next = temp;
        free(temp);
        return 0;
    }
    return Rimuovi_Giocatore(&(*lista)->next, tid);
}

//Definisco una funzione che conta il numero di giocatori
int Numero_Giocatori(Lista_Giocatori lista) {
    //Primo caso base
    if (lista == 0)
        return 0;
    //Secondo caso base
    if (lista == NULL)
        return 0;
    //Caso ricorsivo
    return 1 + Numero_Giocatori(lista->next);
}

//Definisco una funzione per cercare Giocatori nella lista
int CercaUtente (Lista_Giocatori lista, char* utente) {
    //Caso in cui la lista sia vuota
    if (lista == NULL) 
        return 1;
    //Cerco nome utente
    if (strcmp(utente, lista->nome) == 0) {
        return 0;
    }
    //Cerca il prossimo nome utente nella lista
    return CercaUtente(lista->next, utente);
}