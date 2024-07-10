//Struttura dei elementi interni alla matrice
typedef struct lett{
    char lettera;
    int visitato;
}Lettera;


//Funzione che crea la matrice randomica
Lettera** Crea_Matrix();

//Funzione che crea la matrice da un file
void Carica_Matrix_File(FILE* file, Lettera** matrice, int offset);

//Funzione che crea la matrice randomica
void Genera_Matrix(Lettera** matrice, int seed);

//Funzione che cerca la parola digitata dall'utente
int Controlla_Parola(Lettera** matrice, char* parola_utente);