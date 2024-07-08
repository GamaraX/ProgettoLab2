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
#define HELP_MESSAGE "I comandi a disposizione sono i seguenti:\n Tramite il comando 'registra_utente' seguito da un nome utente, è possibile registrarsi e cominciare a giocare.\n Dopo che la partita è cominciata, è possibile richiedere la matrice della partita tramite il comando 'matrice'. Se viene inviato il comando quando la partita non è in corso, verrà indicato il tempo rimanente prima che la partita cominci.\n Per cominciare a cercare parole trovate nella matrice, è possibile inviare il comando 'p' seguito dalla parola trovata nella matrice.\n Tramite il comando 'login_utente' è possibile riconnettersi alla partita dopo essere stati disconnessi per inattività.\n È possibile mostrare i messaggi della bacheca inviati dagli altri giocatori tramite il messaggio 'show_msg', mentre per scrivere un messaggio tramite il comando 'msg' seguito dal testo del messaggio.\n Con il comando 'fine' si esce dal gioco. È possibile digitare il comando 'aiuto' in qualsiasi momento per un elenco dei commandi disponibili\n"

//Definizione funzione che invia messaggi
int Caronte(int fd, char* msg, char carattere);

//Definizione funzione che riceve messaggi
char* Ade(int fd, char* carattere);