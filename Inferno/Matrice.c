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
#include "../Purgatorio/ListaClient.h"
#include "../Purgatorio/Matrice.h"

Lettera** Crea_Matrix() {
    Lettera** mtx = (Lettera**) malloc(4*sizeof(Lettera*));
    for (int i = 0; i < 4; i++) {
        mtx[i] = (Lettera*) malloc(4*sizeof(Lettera));
    }
    return mtx;
}

void Carica_Matrix_File(FILE* file, Lettera** matrice, int offset) {
    char* token;
    //Prendo e apro il file
    FILE* tempfd = fopen(file,"r");
    if (tempfd == NULL) {
        perror("Errore apertura file");
        return NULL;
    }
    
    
}

int Controlla_Parola(char* parola_utente) {

}