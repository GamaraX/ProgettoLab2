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
#include "../Purgatorio/Dizionario.h"


#define MAX_PAROLE 279875
#define MAX_LUNGHEZZA 100



int Carica_Dizionario(const char *nomeFile, char *parole[]) {
    char buffer[MAX_LUNGHEZZA];
    int conteggio = 0;
    FILE *file = fopen(nomeFile, "r");
    if (!file) {
        perror("Errore nell'aprire il file");
        return -1; // Restituisci -1 in caso di errore
    }
    
    // Leggi le parole dal file
    while (fscanf(file, "%s", buffer) == 1 && conteggio < MAX_PAROLE) {
        
        parole[conteggio] = malloc(strlen(buffer) + 1);
        if (!parole[conteggio]) {
            perror("Errore di allocazione della memoria");
            fclose(file);
            return -1; // Restituisci -1 in caso di errore
        }
        strcpy(parole[conteggio], buffer);
        conteggio++;
    }
    

    fclose(file);
    return conteggio; // Restituisci il numero di parole caricate
}


int Ricerca_Binaria_Dizionario(char* parole[], int conteggio, const char *parolaDaCercare) {
    int sinistra = 0, destra = conteggio - 1;
    int trovato = 0;

    while (sinistra <= destra) {
        int centro = (sinistra + destra) / 2;
        
        int confronto = strcmp(parole[centro], parolaDaCercare);
        
        if (confronto == 0) {
            trovato = 1; // Parola trovata
            break; // Esci dal ciclo se trovata
        } else if (confronto < 0) {
            sinistra = centro + 1; // Cerca a destra
        } else {
            destra = centro - 1; // Cerca a sinistra
        }
    }

    return trovato; // Restituisce 1 se trovata, 0 altrimenti
}

void Dealloca_Dizionario(char* parole[], int conteggio) {
    for(int i=0; i<conteggio; i++){
        free(parole[i]);
    }
}