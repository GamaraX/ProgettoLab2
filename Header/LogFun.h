#include <pthread.h>

//- Formato di Log:
//nome_utente;Registrato/Cancellato/Inserimento/Classifica;" "/parola_digitata/classifica con punteggio 

//Funzione che inizializza i log
void InizializzaMutexLog();

//Funzione che scrive nel file di log
void ScriviLog(char* utente, char* azione, char* testo);