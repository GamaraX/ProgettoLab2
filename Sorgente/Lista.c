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
    //Se la lista è vuota, restituisci la stringa vuota
    if (*lista == NULL) {
        pthread_mutex_unlock(&lista_conc->lock);
        return "";
    }
    //Controllo se il nome utente coincide
    if ((*lista)->nome == nome_utente) {
        //Faccio puntare alla testa della lista
        Giocatore* temp = *lista;
        *lista = (*lista)->next;
        //Copio il nome eliminato
        tmpusername = strdup(temp->nome);
        //Libero la memoria
        free(temp->nome);
        free(temp);
        pthread_mutex_unlock(&lista_conc->lock);
        return tmpusername;
    }
    //Creo un puntatore al precedente nodo della lista giocatori
    Lista_Giocatori prev = *lista;
    Lista_Giocatori curr = (*lista)->next;

    //Continuo fino a che non arrivo alla fine della lista
    while (curr != NULL) {
        //Se ho trovato il nome utente, taglio la lista e la cucio facendo puntare il precedente nodo della lista con quello successivo al corrente
        if (curr->nome == nome_utente) {
            prev->next = curr->next;
            //Copio il nome eliminato
            tmpusername = strdup(curr->nome);
            //Libero la memoria       
            free(curr->nome);
            free(curr);
            pthread_mutex_unlock(&lista_conc->lock);
            return tmpusername;
        }
        prev = curr;
        curr = curr->next;
    }
    //Non ho trovato il nome utente nella lista
    pthread_mutex_unlock(&lista_conc->lock);
    return "";
}

//Funzione per rimuovere i client tramite il fd
Lista_FDCLIENT Rimuovi_FD (Lista_FDCLIENT lista, int fd_client) {
    Lista_FDCLIENT listafd = lista;
    //Se la lista è vuota, restituisco la lista fd senza cambiamenti
    if (listafd == NULL) {
        return listafd;
    }
    //Controllo se il fd coincide
    if (listafd->fd_client == fd_client) {
        printf("Client eliminato\n");
        listafd = listafd->next;
        return listafd;
    }
    //Creo un puntatore al precedente nodo della lista fd
    FDCLIENT* prev = lista;
    FDCLIENT* curr = lista->next;

    //Continuo fino a che non arrivo alla fine della lista
    while (curr != NULL) {
        //Se ho trovato il fd, taglio la lista e la cucio facendo puntare il precedente nodo della lista con quello successivo al corrente
        if (curr->fd_client == fd_client) {
            prev->next = curr->next;
            //Libero la memoria        
            free(curr);
            return listafd;
        }
        prev = curr;
        curr = curr->next;
    }
    //Non ho trovato il fd nella lista
    return listafd;
}

//Funzione per contare il numero di Giocatori
int Numero_Giocatori_Loggati(Lista_Giocatori_Concorrente* lista_conc) {
    pthread_mutex_lock(&lista_conc->lock);
    Lista_Giocatori lista = lista_conc->lista;
    //Inizializzo contatore
    int count = 0;
    //Fino a che la lista non è vuota, la scorro e incremento il contatore
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
    //Fino a che la lista non è vuota, controllo se il nome utente coincide con quello nella lista
    while (lista != NULL) {
        if (strcmp(lista->nome, utente) == 0) {
            pthread_mutex_unlock(&lista_conc->lock);
            return 0;
        }
        lista = lista->next;
    }
    //Non ho trovato il nome utente dentro la lista
    pthread_mutex_unlock(&lista_conc->lock);
    return 1;
}

//Funzione che recupera il nome utente di un giocatore
Lista_Giocatori RecuperaUtente(Lista_Giocatori_Concorrente* lista_conc, char* utente) {
    pthread_mutex_lock(&lista_conc->lock);
    Lista_Giocatori head = lista_conc->lista;
    Lista_Giocatori lista = lista_conc->lista;
    //Fino a che la lista non è vuota, controllo se il nome utente coincide con quello nella lista
    while (lista != NULL) {
        if (strcmp(lista->nome, utente) == 0) {
            pthread_mutex_unlock(&lista_conc->lock);
            return lista;
        }
        lista = lista->next;
    }
    lista_conc->lista = head;
    //Non ho trovato il nome utente dentro la lista
    pthread_mutex_unlock(&lista_conc->lock);
    return NULL;
}

//Funzione che cerca le parole nella lista delle parole dell'utente
int Cerca_Parola(Lista_Parole lista_parole, char* parola_proposta) {
    Lista_Parole temp = lista_parole;
    //Fino a che la lista non è vuota, controllo se la parola dell'utente è già stata trovata e inserita da lui o meno
    while (temp != NULL) {
        if (strcmp(temp->parola, parola_proposta) == 0) {
            return 0;
        }
        temp = temp->next;
    }
    //L'utente non ha mai inserito questa parola
    return 1;
}

// Funzione per stampare la lista di fd_client
void Stampa_FDCLIENT(Lista_FDCLIENT lista) {
    FDCLIENT* current = lista;
    while (current != NULL) {
        printf("fd_client: %d\n", current->fd_client);
        fflush(0);
        current = current->next;
    }
}