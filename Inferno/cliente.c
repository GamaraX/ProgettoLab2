//Definizione Librerie
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
#include "../Purgatorio/macro.h"
#include "../Purgatorio/Protocolli.h"
#include "../Purgatorio/ListaClient.h"
#include "../Purgatorio/Matrice.h"

int main(int argc, char* argv[]) {
    //                   int retvalue;
    //controllo se il numero di parametri passati è giusto
    if (argc != 3) {
        perror("Numero sbagliato di parametri passati!");
        exit(EXIT_SUCCESS);
    }

    //controllo se il secondo parametro è il nome del server...?
    if (strlen(argv[1]) < 9 || strlen(argv[1]) > 15) {
        perror("Errore nome server");
        exit(EXIT_SUCCESS);
    }

    //controllo se il terzo parametro è la porta del server (intero)
    if (atoi(argv[2]) == 0) {
        perror("Errore porta server");
        exit(EXIT_SUCCESS);
    }

    //creo l'identificatore per il socket, salvo e casto come intero la porta del server 
    int fd_server, porta_server = atoi(argv[2]);

    //salvo il nome del server   ???
    char* nome_server = argv[1];

    //creo una struct con le informazioni del server  
    struct sockaddr_in info_server;
    info_server.sin_family = AF_INET; 
    info_server.sin_port = htons(porta_server);
    info_server.sin_addr.s_addr = inet_addr(nome_server);

    //creo la socket del server e alloco il file descriptor del server 
    SYSC(fd_server, socket(AF_INET, SOCK_STREAM, 0),"Errore Socket");

    //ciclo e cerco di connettermi al server; se non riesco a connettermi perchè il server non è pronto, aspetto e riprovo a connettermi.
    //altrimenti esco e chiudo perchè c'è un altro tipo di errore
    while (connect(fd_server, (struct sockaddr *)&info_server, sizeof(info_server))) {
        if (errno == ENOENT)
            sleep(1);
        else { 
            perror("Errore nella connect");
            exit(errno);
        }
    }

    //ricevo i messaggi che l'utente invia come input al client, che poi comunicherà al server
    while(1) {
        ssize_t nread;

        //alloco la quantità di caratteri massimi possibili, contando anche la chat di gioco
        char* cmz = malloc(134*sizeof(char));
        //leggo il messaggio di input che l'utente scrive (d)al client
        SYSC(nread, read(STDIN_FILENO,cmz, 134), "Errore Read");

        //ciclo per controllare se tutti i caratteri sono lettere o numeri -> controllo per registra_utente
        /*for (int i = 0; i < strlen(cmz)-1; i++) {
            if(isdigit(cmz[i]) == 0 || isalpha(cmz[i] == 0)) {
                printf("Carattere non accettato");
                return 0;
            }
        }
        */
        //gestione errori su cosa viene passata come stringa (caratteri validi, se ci sono caratteri e non solo spazi,) (isdigit() e isalpha())
        if(strcmp(cmz, "aiuto\n") == 0) {
            printf(HELP_MESSAGE);
            continue;
        }
        if (strcmp(cmz, "matrice\n") == 0) {
            printf("a");
            continue;
        }
        if (strcmp(cmz, "fine\n") == 0) {
            Caronte(fd_server, "Chiusura client", MSG_FINE);
            close(fd_server);
            exit(EXIT_SUCCESS);
            printf("Comando fine");
            return 0;
        }
        if (strcmp(cmz, "cancella_registrazione\n") == 0) {
            printf("fai qualcosa");
            continue;
        }
        if (strcmp(cmz, "show-msg\n") == 0) {
            printf("fai qualcosa");
            continue;
        }

        //sezione della tokenizzazione
        char* token;
        token = strtok(cmz, " ");
        if (strcmp(token, "registra_utente") == 0) {
            token = strtok(NULL, "\n");
            //printf("return 0\n");         DEBUGG
            Caronte(fd_server, token,MSG_REGISTRA_UTENTE);
            char type = 'd';
            token = Ade(fd_server,&type);
            printf("%s\n",token);
            continue;
        }
        if (strcmp(token, "p") == 0) {
            strtok(NULL, "\n");
            printf("fai qualcosa");
            continue;
        }
        if(strcmp(token, "login_utente") == 0) {
            strtok(NULL, "\n");
            printf("fai qualcosa");
            continue;
        }

    }

}

