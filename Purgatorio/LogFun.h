#include <pthread.h>

//Funzione che inizializza i log
void InizializzaMutexLog();
//Funzione che cerca nel file di log
char* CercaLog(char* azione);
//Funzione che scrive nel file di log
void ScriviLog(char* utente, char* azione, char* testo);