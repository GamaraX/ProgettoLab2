#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define MAX_MESSAGES 8

typedef struct messaggio
{
    char *messaggio;
    char *mittente;
} Messaggio;

Messaggio bacheca[MAX_MESSAGES];
int messaggi_inseriti = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void inserisci_messaggio(char *messaggio, char *mittente)
{
    pthread_mutex_lock(&lock);

    // Se abbiamo raggiunto il limite massimo, rimuoviamo il messaggio piÃ¹ vecchio
    if (messaggi_inseriti >= MAX_MESSAGES)
    {
        free(bacheca[0].messaggio);
        free(bacheca[0].mittente);

        // Spostiamo i messaggi rimanenti verso il basso
        for (int i = 1; i < MAX_MESSAGES; i++)
        {
            bacheca[i - 1] = bacheca[i];
        }

        messaggi_inseriti--; // Riduciamo il conteggio
    }

    // A questo punto possiamo inserire il nuovo messaggio
    bacheca[messaggi_inseriti].messaggio = malloc(strlen(messaggio) + 1);
    strcpy(bacheca[messaggi_inseriti].messaggio, messaggio);

    bacheca[messaggi_inseriti].mittente = malloc(strlen(mittente) + 1);
    strcpy(bacheca[messaggi_inseriti].mittente, mittente);

    messaggi_inseriti++;

    pthread_mutex_unlock(&lock);
}

Messaggio *leggi_messaggi(int *num_messaggi)
{
    pthread_mutex_lock(&lock);

    // Alloca memoria per l'array di messaggi da restituire
    Messaggio *messaggi_letti = malloc(messaggi_inseriti * sizeof(Messaggio));
    if (messaggi_letti == NULL)
    {
        pthread_mutex_unlock(&lock);
        return NULL; // Gestione dell'errore
    }

    // Copia i messaggi dalla bacheca
    for (int i = 0; i < messaggi_inseriti; i++)
    {
        messaggi_letti[i].messaggio = malloc(strlen(bacheca[i].messaggio) + 1);
        strcpy(messaggi_letti[i].messaggio, bacheca[i].messaggio);

        messaggi_letti[i].mittente = malloc(strlen(bacheca[i].mittente) + 1);
        strcpy(messaggi_letti[i].mittente, bacheca[i].mittente);
    }

    *num_messaggi = messaggi_inseriti; // Restituisci il numero di messaggi letti
    pthread_mutex_unlock(&lock);

    return messaggi_letti; // Restituisce l'array di messaggi
}

void libera_messaggi(Messaggio *messaggi, int num_messaggi)
{
    for (int i = 0; i < num_messaggi; i++)
    {
        free(messaggi[i].messaggio);
        free(messaggi[i].mittente);
    }
    free(messaggi);
}