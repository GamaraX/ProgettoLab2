#include <stdio.h>

//Struttura dei elementi interni alla matrice
typedef struct lett{
    char* lettera;
    int visitato;
}Lettera;


//Funzione che crea la matrice randomica
Lettera** Crea_Matrix();

//Funzione che stampa la matrice
void Stampa_Matrix(Lettera** matrice);

//Funzione che carica la matrice da una stringa
void Carica_Matrix_Stringa(Lettera** matrice, char* stringa);

//Funzione che crea la matrice da un file
void Carica_Matrix_File(FILE* file, Lettera** matrice);

//Funzione che crea la matrice randomica
void Genera_Matrix(Lettera** matrice, int seed);

//Funzione che cerca la parola digitata dall'utente nella matrice
int Controlla_Parola_Matrice(Lettera** matrice, char* parola_utente);

//Funzione che esegue la DFS
int DFS_Matrix(Lettera** matrice, char* parola_utente, int pos, int riga, int colonna);

//Funzione che dealloca la matrice, in particolare dealloca tutti i campi delle lettere
void Libera_Matrix(Lettera** matrice);
