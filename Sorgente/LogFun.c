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
    fflush(NULL);
    //Chiudo il file
    fclose(tempfd);
    pthread_mutex_unlock(&log_mutex);
}
