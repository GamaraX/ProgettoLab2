//Struttura del messaggio inviato nella bacheca
typedef struct messaggio {
    char *messaggio;
    char *mittente;
} Messaggio;

//Funzione per inserire messaggio nella bacheca
void inserisci_messaggio(char *messaggio, char *mittente);
//Funzione per leggere la bacheca
Messaggio *leggi_messaggi(int *num_messaggi);
//Funzione per liberare i messaggi dalla memoria
void libera_messaggi(Messaggio *messaggi, int num_messaggi);