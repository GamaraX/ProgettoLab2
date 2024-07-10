// Definizione Librerie
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
#include "../Purgatorio/LogFun.h"

// Inizializzo la lista concorrente
void Inizializza_Lista(Lista_Giocatori_Concorrente* lista_conc) {
    lista_conc->lista = NULL;
    pthread_mutex_init(&lista_conc->lock, NULL);
}

// Elimino la lista concorrente
void Elimina_Lista(Lista_Giocatori_Concorrente* lista_conc) {
    pthread_mutex_destroy(&lista_conc->lock);
}

// Definisco funzione che aggiunge giocatori
void Aggiungi_Giocatore(Lista_Giocatori_Concorrente* lista_conc, char* nome, int fd) {
    pthread_mutex_lock(&lista_conc->lock);
    // Creo un Giocatore nella lista dei Giocatori
    Giocatore* i = (Giocatore*) malloc(sizeof(Giocatore));
    // Associo il thread
    i->thread = pthread_self();
    // Associo il nome
    i->nome = (char*) malloc(strlen(nome) + 1);
    strcpy(i->nome, nome);
    // Assegno i punti (che sono 0 all'inizio della partita)
    i->punti = 0;
    // Associo il fd
    i->fd_client = fd;
    i->loggato = 0;
    // Faccio puntare alla testa della lista
    i->next = lista_conc->lista;
    lista_conc->lista = i;
    pthread_mutex_unlock(&lista_conc->lock);
    return;
}

char* Rimuovi_Giocatore(Lista_Giocatori_Concorrente* lista_conc, pthread_t tid) {
    pthread_mutex_lock(&lista_conc->lock);
    char* tmpusername;
    Lista_Giocatori* lista = &lista_conc->lista;
    // Caso base
    if (*lista == NULL) {
        pthread_mutex_unlock(&lista_conc->lock);
        return "";
    }
    // Controllo se il tid coincide
    if ((*lista)->thread == tid) {
        printf("Giocatore eliminato\n");
        Giocatore* temp = *lista;
        *lista = (*lista)->next;
        tmpusername = strdup(temp->nome);
        free(temp->nome);
        free(temp);
        pthread_mutex_unlock(&lista_conc->lock);
        return tmpusername;
    }
    // Cerco il prossimo tid nella lista
    Lista_Giocatori prev = *lista;
    Lista_Giocatori curr = (*lista)->next;
    while (curr != NULL) {
        if (curr->thread == tid) {
            prev->next = curr->next;
            tmpusername = strdup(curr->nome);            
            free(curr->nome);
            free(curr);
            pthread_mutex_unlock(&lista_conc->lock);
            return tmpusername;
        }
        prev = curr;
        curr = curr->next;
    }
    pthread_mutex_unlock(&lista_conc->lock);
    return "";
}

// Definisco una funzione che conta il numero di giocatori
int Numero_Giocatori(Lista_Giocatori_Concorrente* lista_conc) {
    pthread_mutex_lock(&lista_conc->lock);
    Lista_Giocatori lista = lista_conc->lista;
    int count = 0;
    while (lista != NULL) {
        count++;
        lista = lista->next;
    }
    pthread_mutex_unlock(&lista_conc->lock);
    return count;
}

// Definisco una funzione per cercare Giocatori nella lista
int CercaUtente(Lista_Giocatori_Concorrente* lista_conc, char* utente) {
    pthread_mutex_lock(&lista_conc->lock);
    Lista_Giocatori lista = lista_conc->lista;
    while (lista != NULL) {
        if (strcmp(lista->nome, utente) == 0) {
            pthread_mutex_unlock(&lista_conc->lock);
            return 0;
        }
        lista = lista->next;
    }
    pthread_mutex_unlock(&lista_conc->lock);
    return 1;
}

Lista_Giocatori RecuperaUtente(Lista_Giocatori_Concorrente* lista_conc, char* utente) {
    pthread_mutex_lock(&lista_conc->lock);
    Lista_Giocatori head = lista_conc->lista;
    Lista_Giocatori lista = lista_conc->lista;
    while (lista != NULL) {
        if (strcmp(lista->nome, utente) == 0) {
            pthread_mutex_unlock(&lista_conc->lock);
            return lista;
        }
        lista = lista->next;
    }
    lista_conc->lista = head;
    pthread_mutex_unlock(&lista_conc->lock);
    return NULL;
}
