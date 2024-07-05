//Protocollo di Comunicazione
#define MSG_OK 'K'
#define MSG_ERR 'E'
#define MSG_REGISTRA_UTENTE 'R'
#define MSG_MATRICE 'M'
#define MSG_TEMPO_PARTITA 'T'
#define MSG_TEMPO_ATTESA 'A'
#define MSG_PAROLA 'W'
#define MSG_PUNTI_FINALI 'F'
#define MSG_PUNTI_PAROLA 'P'
#define MSG_CANCELLA_UTENTE 'D'
#define MSG_LOGIN_UTENTE 'L'
#define MSG_POST_BACHECA 'H'
#define MSG_SHOW_BACHECA 'S'
#define MSG_FINE 'X'
#define HELP_MESSAGE "I comandi a disposizione sono i seguenti:"

//Definizione funzione che invia messaggi
int Caronte(int fd, char* msg, char carattere);

//Definizione funzione che riceve messaggi
char* Ade(int fd, char* carattere);