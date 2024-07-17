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

//Funzione che crea la matrice randomica
Lettera** Crea_Matrix() {
    //Alloco lo spazio per le righe della matrice
    Lettera** mtx = (Lettera**) malloc(4 * sizeof(Lettera*));
    for (int i = 0; i < 4; i++) {
        //Alloco lo spazio per le colonne della matrice
        mtx[i] = (Lettera*) malloc(4 * sizeof(Lettera));
        for (int j = 0; j < 4; j++) {
            //Alloco lo spazio per il carattere e pongo il parametro visitato a 0(non visitato)
            mtx[i][j].lettera = (char*) malloc(3 * sizeof(char));
            mtx[i][j].visitato = 0;
        }
    }
    return mtx;
}

//Funzione che stampa la matrice
void Stampa_Matrix(Lettera** matrice) {
    printf("\n");
    fflush(0);
    for (int i = 0; i < 4; i++) {
        printf("%s | %s | %s | %s\n", matrice[i][0].lettera, matrice[i][1].lettera, matrice[i][2].lettera, matrice[i][3].lettera);
        fflush(0);
    }
    return;
}

//Funzione che carica la matrice da una stringa
void Carica_Matrix_Stringa(Lettera** matrice, char* stringa) {
    //Tokenizzo sui caratteri della stringa
    char* token;
    token = strtok(stringa, " ");
    for(int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            //Assegno alla stringa i caratteri salvati in token
            strcpy(matrice[i][j].lettera, token);
            token = strtok(NULL, " ");
        }
    }
}

//Funzione che crea la matrice da un file
void Carica_Matrix_File(FILE* file, Lettera** matrice) {
    char stringatmp[48];
    
    //Controllo se il file esiste o ci sono errori
    if (file == NULL) {
        perror("Errore apertura file");
        return;
    }
    //Controllo se sono arrivato alla fine del file. In tal caso, torno all'inizio del file e ricomincio a caricare le matrici dalla prima
    if(fgets(stringatmp,sizeof(stringatmp), file) == NULL) {
        printf("Fine file\n");
        fflush(0);
        fseek(file, 0, SEEK_SET);
        fgets(stringatmp,sizeof(stringatmp), file);
    }
    //Carico la matrice dal file
    Carica_Matrix_Stringa(matrice, stringatmp);
    
}

//Funzione che crea la matrice randomica
void Genera_Matrix(Lettera** matrice, int seed) {
    //Creo abbastanza spazio per memorizzare la matrice su una stringa
    char stringa[64] = "";
    for(int i = 0; i < 16; i++) {
        //Genero casualmente un carattere dalla A alla Z
        char lett;
        lett = (rand()%(90-65+1))+65;
        //Nel caso il carattere generato sia Q, inserisco nella stringa il carattere Qu; altrimenti lo assegno semplicemente
        if (lett == 'Q' )
            strcat(stringa, "Qu");
        else {
            char temp[2] = {lett, '\0'};
            strcat(stringa, temp);
        }
        //Inserisco uno spazio tra un carattere della stringa e l'altro
        if (i < 15) {
            strcat(stringa, " ");
        }
    }
    //Carico la matrice dalla stringa
    Carica_Matrix_Stringa(matrice, stringa);
}

//Funzione che cerca la parola digitata dall'utente nella matrice
int Controlla_Parola_Matrice(Lettera** matrice, char* parola_utente) {
    //Alloco lo spazio per memorizzare la parola dell'utente in una variabile temporanea
    char* temppparola = malloc((strlen(parola_utente)+1)*sizeof(char));
    strcpy(temppparola, parola_utente);
    //Rendo la parola dell'utente in maiuscolo per poterla cercare nella matrice
    for (int i = 0; i < strlen(parola_utente)-1 ; i++) {
        temppparola[i] = toupper(parola_utente[i]);
    }
    char lettiniz[3];
    //Controllo se la prima lettera della parola dell'utente è Q oppure uno degli altri caratteri. In tal caso, memorizzo la lettera Qu
    if (temppparola[0] == 'Q') {
        strcpy(lettiniz, "Qu");
    }
    else {
        lettiniz[0] = temppparola[0];
        lettiniz[1] = '\0';
    }
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            //Se la prima lettera della parola dell'utente non è nemmeno uguale al primo carattere con cui lo confronto, non eseguo nemmeno la ricerca DFS
            if (strcmp(matrice[i][j].lettera, lettiniz) == 0) {
                //Se il controllo è stato passato, eseguo la ricerca nella matrice
                if (DFS_Matrix(matrice, temppparola, 0, i, j) == 1) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

//Funzione che esegue la DFS
int DFS_Matrix(Lettera** matrice, char* parola_utente, int pos, int riga, int colonna) {
    //Controllo se sto uscendo fuori dalla matrice
    if (riga < 0 || riga >= 4 || colonna < 0 || colonna >= 4){
        return 0;
    }

    char* lett = malloc(3* sizeof(char));
    //Controllo se la parola pos-esima dell'utente è la parola Q, che in tal caso devo trattare come Qu
    if (parola_utente[pos] == 'Q') {
        strcpy(lett, "Qu");
    } else {
        lett[0] = parola_utente[pos];
        lett[1] = '\0';
    }
    //Se la somma tra la pos e la lunghezza del carattere da cercare è maggiore o uguale della lunghezza della parola dell'utente, ho trovato la parola che cercavo
    if (pos + strlen(lett) >= strlen(parola_utente))
        return 1;

    //Controllo se la lettera pos-esima della parola utente è presente nella matrice, oppure se l'elemento della matrice è già stato visitato o meno
    if (strcmp(matrice[riga][colonna].lettera, lett) != 0 || matrice[riga][colonna].visitato) {
        free(lett);
        return 0;
    }
    //Pongo come visitato la lettera in cui sono al momento
    matrice[riga][colonna].visitato = 1;

    //Chiamate ricorsive per le 4 direzioni possibili
    int trovato = DFS_Matrix(matrice, parola_utente, pos + strlen(lett), riga + 1, colonna)
               || DFS_Matrix(matrice, parola_utente, pos + strlen(lett), riga - 1, colonna)
               || DFS_Matrix(matrice, parola_utente, pos + strlen(lett), riga, colonna + 1)
               || DFS_Matrix(matrice, parola_utente, pos + strlen(lett), riga, colonna - 1);
    matrice[riga][colonna].visitato = 0;

    free(lett);
    return trovato;
}

//Funzione che svuota la matrice, in particolare svuota tutti i campi delle lettere
void Libera_Matrix(Lettera** matrice) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            strcpy(matrice[i][j].lettera, "");
        }
    }
}
