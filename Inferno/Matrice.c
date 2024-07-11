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

void Stampa_Matrix(Lettera** matrice) {
    for (int i = 0; i < 4; i++) {
            printf("%s | %s | %s | %s\n", matrice[i][0].lettera, matrice[i][1].lettera, matrice[i][2].lettera, matrice[i][3].lettera);
            fflush(0);
    }
    return;
}

void Carica_Matrix_File(char* file, Lettera** matrice, int* offset) {
    char* token, stringatmp[48];
    //Prendo e apro il file
    FILE* tempfd = fopen(file,"r");
    //Controllo se il file esiste o ci sono errori/corruzioni
    if (tempfd == NULL) {
        perror("Errore apertura file");
        return;
    }
    //Inizio a leggere dalla prima riga ogni lettera fino alla fine della riga
    fseek(tempfd, *offset, SEEK_SET);
    fscanf(tempfd,"%s", stringatmp);
    token = strtok(stringatmp," ");
    //Memorizzo nella matrice la lettera corrispondente dal file
    while(token != NULL) {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                matrice[i][j].lettera = token[0];
                token = strtok(NULL, " ");
            }
        }
    }
    //Imposto l'offset alla prossima riga
    *offset = fseek(tempfd, 0, SEEK_SET);
    fclose(tempfd);
}

void Genera_Matrix(Lettera** matrice, int seed) {
    //Pongo il seed della funzione rand al valore seed passato come argomento
    srand(seed);
    for(int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            matrice[i][j].lettera = (rand()%(90-65))+65;
            if (strcmp(matrice[i][j].lettera, "Q") == 0 )
                matrice[i][j].lettera = "Qu";
        }
    }
}


int Controlla_Parola_Matrice(Lettera** matrice, char* parola_utente) {
    //Cerco nella matrice la lettera pos-esima della parola dell'utente
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if(DFS_Matrix(matrice, parola_utente, 0, i, j) == 1)
                return 1;
        }
    }
    return 0;
}

int DFS_Matrix(Lettera** matrice, char* parola_utente, int pos, int riga, int colonna) {
    if(strlen(parola_utente) == pos)
        return 1;
////////Non controllo veramente se sto uscendo fuori dalla parola -> parola_utente = ""
    char* lett;
    //Controllo se la parola pos-esima dell'utente è la parola Q, che in tal caso devo trattare come Qu
    if (strcmp("Q", parola_utente[pos]) == 0 ) {
        pos += 1;
        lett = "Qu";
    }
    else {
        lett = parola_utente[pos];
    }
    //Controllo se sto uscendo fuori dalla matrice
    if(riga<0 || riga >= 4 || colonna < 0 || colonna >= 4)
        return 0;

    //Controllo se la lettera pos-esima della parola utente è presente nella matrice, oppure se l'elemento della matrice è già stato visitato o meno
    if (strcmp(matrice[riga][colonna].lettera, lett) != 0 || matrice[riga][colonna].visitato) {
       return 0;
    }
    matrice[riga][colonna].visitato = 1;
    int trovato1, trovato2, trovato3, trovato4;
    trovato1 = DFS_Matrix(matrice, parola_utente, pos+1, riga+1, colonna);
    trovato2 = DFS_Matrix(matrice, parola_utente, pos+1, riga-1, colonna);
    trovato3 = DFS_Matrix(matrice, parola_utente, pos+1, riga, colonna+1);
    trovato4 = DFS_Matrix(matrice, parola_utente, pos+1, riga, colonna-1);
    matrice[riga][colonna].visitato = 0;
    return trovato1 || trovato2 || trovato3 || trovato4;
}
