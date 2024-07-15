//
typedef struct messaggio
{
    char *messaggio;
    char *mittente;
} Messaggio;

void inserisci_messaggio(char *messaggio, char *mittente);

Messaggio *leggi_messaggi(int *num_messaggi);

void libera_messaggi(Messaggio *messaggi, int num_messaggi);