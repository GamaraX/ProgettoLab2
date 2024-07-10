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
        for(int j = 0; j < 4; j++) {
            mtx[i][j].visitato = 0;
        }
    }
    return mtx;
}

void Carica_Matrix_File(char* file, Lettera** matrice, int* offset) {
    char* token, stringatmp[48];
    //Prendo e apro il file
    FILE* tempfd = fopen(file,"r");
    if (tempfd == NULL) {
        perror("Errore apertura file");
        return NULL;
    }
    fseek(tempfd, offset, SEEK_SET);
    fscanf(tempfd,"%s", stringatmp);
    token = strtok(stringatmp," ");
    while(token != NULL) {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                matrice[i][j].lettera = token[0];
                token = strtok(NULL, " ");
            }
        }
    }
    *offset = fseek(tempfd, 0, SEEK_SET);
    fclose(tempfd);
}

void Genera_Matrix(Lettera** matrice, int seed) {
    
}


int Controlla_Parola(Lettera** matrice, char* parola_utente) {
    //controlla dal dizionario con un sorting

    //DFS per cercare la parola nella matrice

}