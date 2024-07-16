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
#include "../Header/macro.h"
#include "../Header/Protocolli.h"
#include "../Header/Lista.h"
#include "../Header/Matrice.h"
#include "../Header/LogFun.h"

//Funzione che inizializza la lista a vuota
void Inizializza_Lista(Lista_Giocatori_Concorrente* lista_conc) {
    lista_conc->lista = NULL;
    pthread_mutex_init(&lista_conc->lock, NULL);
}

//Funzione che elimina la lista
void Elimina_Lista(Lista_Giocatori_Concorrente* lista_conc) {
    pthread_mutex_destroy(&lista_conc->lock);
}

//Funzione per aggiungere Giocatori
void Aggiungi_Giocatore(Lista_Giocatori_Concorrente* lista_conc, char* nome, int fd) {
    pthread_mutex_lock(&lista_conc->lock);
    //Creo un Giocatore nella lista dei Giocatori
    Giocatore* i = (Giocatore*) malloc(sizeof(Giocatore));
    //Associo il thread
    i->thread = pthread_self();
    //Associo il nome
    i->nome = (char*) malloc(strlen(nome) + 1);
    strcpy(i->nome, nome);
    //Assegno i punti (che sono 0 all'inizio della partita)
    i->punti = 0;
    //Associo il fd
    i->fd_client = fd;
    //Associo il logging (che all'inizio è zero)
    i->loggato = 0;
    //Faccio puntare alla testa della lista
    i->next = lista_conc->lista;
    lista_conc->lista = i;
    pthread_mutex_unlock(&lista_conc->lock);
    return;
}

//Funzione per rimuovere Giocatori
char* Rimuovi_Giocatore(Lista_Giocatori_Concorrente* lista_conc, char* nome_utente) {
    pthread_mutex_lock(&lista_conc->lock);
    char* tmpusername;
    Lista_Giocatori* lista = &lista_conc->lista;
    // Caso base
    if (*lista == NULL) {
        pthread_mutex_unlock(&lista_conc->lock);
        return "";
    }
    // Controllo se il nome utente coincide
    if ((*lista)->nome == nome_utente) {
        printf("Giocatore eliminato\n");
        Giocatore* temp = *lista;
        *lista = (*lista)->next;
        tmpusername = strdup(temp->nome);
        free(temp->nome);
        free(temp);
        pthread_mutex_unlock(&lista_conc->lock);
        return tmpusername;
    }
    // Cerco il prossimo nome utente nella lista
    Lista_Giocatori prev = *lista;
    Lista_Giocatori curr = (*lista)->next;
    while (curr != NULL) {
        if (curr->nome == nome_utente) {
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

int Rimuovi_FD (Lista_FDCLIENT lista, int fd_client) {
    int tmpfd;
    Lista_FDCLIENT listafd = lista;
    // Caso base
    if (listafd == NULL) {
        return 0;
    }
    // Controllo se il nome utente coincide
    if (listafd->fd_client == fd_client) {
        printf("Client eliminato\n");
        FDCLIENT* temp = listafd;
        listafd = listafd->next;
        tmpfd = temp->fd_client;
        return 1;
    }
    // Cerco il prossimo nome utente nella lista
    FDCLIENT* prev = lista;
    FDCLIENT* curr = lista->next;
    while (curr != NULL) {
        if (curr->fd_client == fd_client) {
            prev->next = curr->next;
            tmpfd = curr->fd_client;            
            free(curr);
            return tmpfd;
        }
        prev = curr;
        curr = curr->next;
    }
    return 1;
}
//Funzione per contare il numero di Giocatori
int Numero_Giocatori_Loggati(Lista_Giocatori_Concorrente* lista_conc) {
    pthread_mutex_lock(&lista_conc->lock);
    Lista_Giocatori lista = lista_conc->lista;
    int count = 0;
    while (lista != NULL) {
        if(lista->loggato == 1) {
            count++;
            lista = lista->next;
        }
    }
    pthread_mutex_unlock(&lista_conc->lock);
    return count;
}

//Funzione per cercare un giocatore con un determinato nome utente
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

//Funzione che recupera il nome utente di un giocatore
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

int Cerca_Parola(Lista_Parole lista_parole, char* parola_proposta) {
    Lista_Parole temp = lista_parole;
    while (temp != NULL) {
        if (strcmp(temp->parola, parola_proposta) == 0) {
            return 0;
        }
        temp = temp->next;
    }
    return 1;
}