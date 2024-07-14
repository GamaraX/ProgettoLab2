
//Funzione che esegue la ricerca binaria nel dizionario per cercare la parola inviata dall'utente
int Ricerca_Binaria_Dizionario(char* parole[], int conteggio, const char *parolaDaCercare);
//Funzione che carica il dizionario in memoria
int Carica_Dizionario(const char *nomeFile, char* parole[]);
//Funzione che Dealloca il dizionario dalla memoria
void Dealloca_Dizionario(char* parole[], int conteggio);
