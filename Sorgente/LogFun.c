//Definizione Librerie
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "../Header/macro.h"
#include "../Header/Protocolli.h"
#include "../Header/Lista.h"
#include "../Header/LogFun.h"

//Inizializzo variabili globali
pthread_mutex_t log_mutex;
char* filelog;

//Funzione che inizializza i log
void InizializzaMutexLog() {
    pthread_mutex_init(&log_mutex, NULL);
}

//Funzione che cerca nel file di log
char* CercaLog(char* azione) {
    pthread_mutex_lock(&log_mutex);
    char* ris_azione = NULL;
    char* linea_file = NULL;
    char* token_azione;
    size_t len = 0;
    ssize_t cmplettura;
    //Apro il file in sola lettura
    FILE* tempfd = fopen("../Eseguibili/Log.txt", "r");
    //Controllo se il file esiste o ci sono errori/corruzioni
    if (tempfd == NULL) {
        perror("Errore apertura file");
        pthread_mutex_unlock(&log_mutex);
        return NULL;
    }
    //Fino a che non arrivo alla fine del file, leggo il secondo parametro (che è l'azione eseguita dal client) e controllo se è l'azione che sto cercando
    while ((cmplettura = getline(&linea_file, &len, tempfd)) != -1) {
        token_azione = strtok(linea_file, ";");
        token_azione = strtok(NULL, ";");
        if (token_azione != NULL && strcmp(token_azione, azione) == 0) {
            ris_azione = strdup(azione); 
            break;
        }
    }
    free(linea_file);
    fclose(tempfd);
    pthread_mutex_unlock(&log_mutex);
    return ris_azione;
}

//Funzione che scrive nel file di log
void ScriviLog(char* utente, char* azione, char* testo) {
    //printf("%s %s %s\n", utente, azione, testo);              Debugging
    pthread_mutex_lock(&log_mutex);
    //Apro il file in append
    FILE* tempfd = fopen("../Eseguibili/Log.txt", "a");
    //Controllo se il file esiste o ci sono errori
    if (tempfd == NULL) {
        perror("Errore apertura file");
        pthread_mutex_unlock(&log_mutex);
        return;
    }
    //Scrivo nel log la terna in formato nome utente;azione eseguita;testo (viene memorizzata la parola o la classifica, altrimenti è un white space)
    fprintf(tempfd, "%s;%s;%s\n", utente, azione, testo);
    //Chiudo il file
    fclose(tempfd);
    pthread_mutex_unlock(&log_mutex);
}
