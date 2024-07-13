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
//#todo fix, usare funzione Carica_Matrix_Stringa e usa la stringa che leggi
//#todo fixare offest
void Carica_Matrix_File(char* file, Lettera** matrice, int* offset) {
    char* token, stringatmp[48];
    //Prendo e apro il fileù
    FILE* tempfd = fopen(file,"r");
    //Controllo se il file esiste o ci sono errori/corruzioni
    if (tempfd == NULL) {
        perror("Errore apertura file");
        return;
    }
    //Inizio a leggere dalla prima riga ogni lettera fino alla fine della riga
    fseek(tempfd, 0, SEEK_SET);

    fgets(stringatmp,sizeof(stringatmp), tempfd);
                            //printf("%s\n", stringatmp);
                            //fflush(0);
    token = strtok(stringatmp," ");
                            //printf("%s\n", stringatmp);
                            //fflush(0);
    //Memorizzo nella matrice la lettera corrispondente dal file
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                /*
                printf("%s", token);
                fflush(0);
                */
               strcpy(matrice[i][j].lettera, token);
                token = strtok(NULL, " ");
            }
        }
    //Imposto l'offset alla prossima riga
    /*if (ftell(tempfd) == EOF) {
        *offset = 0;
    }
    else{
        *offset = ftell(tempfd);
    }*/
    //*offset = ftell(tempfd);
    fclose(tempfd);
}
//#todo fixare genera matrix ma in realta' devi solo generare una stringa di 16 caratteri separati da spazio (controllare la Q come gia` fai) e poi chiamare Carica_Matrix_Stringa

void Genera_Matrix(Lettera** matrice, int seed) {
    //Pongo il seed della funzione rand al valore seed passato come argomento
    srand(seed);
    for(int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            char lett;
            lett = (rand()%(90-65+1))+65;
            if (lett == 'Q' )
                strcpy(matrice[i][j].lettera, "Qu");
            else
                strcpy(matrice[i][j].lettera,lett);
            printf("%s", matrice[i][j].lettera);
            fflush(0);
        }
    }
}

//#todo attenzione perche' c'e' un'ottimizzazione che puoi fare: far partire la ricerca solo se la prima lettera coincide (di sicuro non trovi la parole ALBERO partendo dalla lettera E)
int Controlla_Parola_Matrice(Lettera** matrice, char* parola_utente) {
    // Cerco nella matrice la lettera pos-esima della parola dell'utente
    char lettiniz[3];
    if (parola_utente[0] == 'Q') {
        strcpy(lettiniz, "Qu");

    }
    else {
        lettiniz[0] = parola_utente[0];
        lettiniz[1] = '\0';
    }
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                // Uso strcmp per confrontare le stringhe
                if (strcmp(matrice[i][j].lettera, lettiniz) == 0) {
                    if (DFS_Matrix(matrice, parola_utente, 0, i, j) == 1) {
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

    printf("Controllo matrice[%d][%d]: %s con %s\n", riga, colonna, matrice[riga][colonna].lettera, lett);
    fflush(0);
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

//#todofatta dealloca la matrice, prende in ingresso Lettera** e fa la free di tutti i campi (ritorna void)
void Libera_Matrix(Lettera** matrice) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            free(matrice[i][j].lettera);
        }
    }
}
