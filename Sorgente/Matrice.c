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
#include "../Header/Lista.h"
#include "../Header/Matrice.h"

Lettera** Crea_Matrix() {
    Lettera** mtx = (Lettera**) malloc(4 * sizeof(Lettera*));
    for (int i = 0; i < 4; i++) {
        mtx[i] = (Lettera*) malloc(4 * sizeof(Lettera));
        for (int j = 0; j < 4; j++) {
            mtx[i][j].lettera = (char*) malloc(3 * sizeof(char));
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

void Carica_Matrix_Stringa(Lettera** matrice, char* stringa) {
    char* token;
    token = strtok(stringa, " ");
    for(int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            strcpy(matrice[i][j].lettera, token);
            token = strtok(NULL, " ");
        }
    }
}

void Carica_Matrix_File(FILE* file, Lettera** matrice) {
    char stringatmp[48];
    
    //Controllo se il file esiste o ci sono errori/corruzioni
    if (file == NULL) {
        perror("Errore apertura file");
        return;
    }

    if(fgets(stringatmp,sizeof(stringatmp), file) == NULL) {
        printf("Fine file\n");
        fflush(0);
        fseek(file, 0, SEEK_SET);
        fgets(stringatmp,sizeof(stringatmp), file);
    }

    Carica_Matrix_Stringa(matrice, stringatmp);
    
}

void Genera_Matrix(Lettera** matrice, int seed) {
    //Pongo il seed della funzione rand al valore seed passato come argomento
    char stringa[64] = "";

    for(int i = 0; i < 16; i++) {
        //
        char lett;
        lett = (rand()%(90-65+1))+65;

        if (lett == 'Q' )
            strcat(stringa, "Qu");
        else {
            char temp[2] = {lett, '\0'};
            strcat(stringa, temp);
        }
        if (i < 15) {
            strcat(stringa, " ");
        }
    }
    Carica_Matrix_Stringa(matrice, stringa);
}

int Controlla_Parola_Matrice(Lettera** matrice, char* parola_utente) {
    // Cerco nella matrice la lettera pos-esima della parola dell'utente
    char* temppparola = malloc((strlen(parola_utente)+1)*sizeof(char));
    strcpy(temppparola, parola_utente);
    for (int i = 0; i < strlen(parola_utente)-1 ; i++) {
        temppparola[i] = toupper(parola_utente[i]);
    }
    char lettiniz[3];
    if (temppparola[0] == 'Q') {
        strcpy(lettiniz, "Qu");
    }
    else {
        lettiniz[0] = temppparola[0];
        lettiniz[1] = '\0';
    }
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                // Uso strcmp per confrontare le stringhe
                if (strcmp(matrice[i][j].lettera, lettiniz) == 0) {
                    if (DFS_Matrix(matrice, temppparola, 0, i, j) == 1) {
                        return 1;
                    }
                }
            }
        }
    return 0;
}


int DFS_Matrix(Lettera** matrice, char* parola_utente, int pos, int riga, int colonna) {
    // Controllo se sto uscendo fuori dalla matrice
    if (riga < 0 || riga >= 4 || colonna < 0 || colonna >= 4){
        return 0;
    }

    char* lett = malloc(3* sizeof(char));
    // Controllo se la parola pos-esima dell'utente è la parola Q, che in tal caso devo trattare come Qu
    if (parola_utente[pos] == 'Q') {
        strcpy(lett, "Qu");
    } else {
        lett[0] = parola_utente[pos];
        lett[1] = '\0';
    }
    if (pos + strlen(lett) >= strlen(parola_utente))
        return 1;

    // Controllo se la lettera pos-esima della parola utente è presente nella matrice, oppure se l'elemento della matrice è già stato visitato o meno
    if (strcmp(matrice[riga][colonna].lettera, lett) != 0 || matrice[riga][colonna].visitato) {
        free(lett);
        return 0;
    }

    matrice[riga][colonna].visitato = 1;

    // Chiamate ricorsive per le 4 direzioni possibili
    int trovato = DFS_Matrix(matrice, parola_utente, pos + strlen(lett), riga + 1, colonna)
               || DFS_Matrix(matrice, parola_utente, pos + strlen(lett), riga - 1, colonna)
               || DFS_Matrix(matrice, parola_utente, pos + strlen(lett), riga, colonna + 1)
               || DFS_Matrix(matrice, parola_utente, pos + strlen(lett), riga, colonna - 1);
    matrice[riga][colonna].visitato = 0;

    free(lett);

    return trovato;
    //return trovato1 || trovato2 || trovato3 || trovato4;
}


void Libera_Matrix(Lettera** matrice) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            strcpy(matrice[i][j].lettera, "");
        }
    }
}
