#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "../Header/Bacheca.h"

#define MAX_MESSAGES 8
//Definisco la grandezza della bacheca
Messaggio bacheca[MAX_MESSAGES];

//Inizializzo variabili globali e mutex
int messaggi_inseriti = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

//Funzione per inserire messaggio nella bacheca
void inserisci_messaggio(char *messaggio, char *mittente) {
    pthread_mutex_lock(&lock);

    //Rimuoviamo il messaggio più vecchio se abbiamo raggiunto il limite massimo
    if (messaggi_inseriti >= MAX_MESSAGES) {
        free(bacheca[0].messaggio);
        free(bacheca[0].mittente);

        //Spostiamo i messaggi rimanenti verso il basso
        for (int i = 1; i < MAX_MESSAGES; i++) {
            bacheca[i - 1] = bacheca[i];
        }
        //Diminuiamo il conteggio dei messaggi inseriti
        messaggi_inseriti--;
    }
    
    //Inserisco il nuovo messaggio e mittente nella bacheca
    bacheca[messaggi_inseriti].messaggio = malloc(strlen(messaggio) + 1);
    strcpy(bacheca[messaggi_inseriti].messaggio, messaggio);

    bacheca[messaggi_inseriti].mittente = malloc(strlen(mittente) + 1);
    strcpy(bacheca[messaggi_inseriti].mittente, mittente);
    //Aumento il conteggio dei messaggi inseriti
    messaggi_inseriti++;

    pthread_mutex_unlock(&lock);
}

//Funzione per leggere la bacheca
Messaggio *leggi_messaggi(int *num_messaggi) {
    pthread_mutex_lock(&lock);

    //Alloco memoria per l'array di messaggi da restituire
    Messaggio *messaggi_letti = malloc(messaggi_inseriti * sizeof(Messaggio));
    //Controllo se c'è almeno un messaggio che è stato postato
    if (messaggi_letti == NULL) {
        pthread_mutex_unlock(&lock);
        return NULL; 
    }

    //Copio i messaggi dalla bacheca
    for (int i = 0; i < messaggi_inseriti; i++) {
        messaggi_letti[i].messaggio = malloc(strlen(bacheca[i].messaggio) + 1);
        strcpy(messaggi_letti[i].messaggio, bacheca[i].messaggio);

        messaggi_letti[i].mittente = malloc(strlen(bacheca[i].mittente) + 1);
        strcpy(messaggi_letti[i].mittente, bacheca[i].mittente);
    }
    //Salvo il numero di messaggi letti
    *num_messaggi = messaggi_inseriti; 
    pthread_mutex_unlock(&lock);
    //Invio l'array di messaggi
    return messaggi_letti; 
}

//Funzione per liberare i messaggi dalla memoria
void libera_messaggi(Messaggio *messaggi, int num_messaggi) {
    for (int i = 0; i < num_messaggi; i++) {
        free(messaggi[i].messaggio);
        free(messaggi[i].mittente);
    }
    free(messaggi);
}