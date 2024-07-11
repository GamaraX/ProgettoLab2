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
#include "../Purgatorio/LogFun.h"
#include "../Purgatorio/Dizionario.h"
//
#define Lunghezza_Parola_Massima 50

int Ricerca_Binaria_Dizionario(char* file, char* parola_utente){
    //Apro il file dizionario
    FILE* tempfd = fopen(file,"r");
    //
    if (tempfd == NULL) {
        perror("Errore apertura file");
        return;
    }
    //
    char linea[Lunghezza_Parola_Massima];
    int inizio, mezzo;
    fseek(tempfd, 0, SEEK_END);
    int fine = ftell(tempfd);
    //
    while(inizio <= fine) {
        mezzo = (inizio-fine) / 2;
        fseek(tempfd, mezzo, SEEK_SET);
        //
        if (mezzo != 0) {
            while(fgetc(tempfd) != '\n' && ftell(tempfd) > 1) 
                fseek(tempfd, -2, SEEK_CUR);
        }

        //
        if(fgets(linea,sizeof(linea), tempfd) == NULL)
            break;
        linea[strcspn(linea, "\n")] = 0;
        //
        if (strcmp(linea, parola_utente) == 0) {
            fclose(tempfd);
            return 1;
        }
        else if (strcmp(linea, parola_utente) < 0)
            inizio = ftell(tempfd);
        else {
            fine = mezzo-1;
        }
    }
    fclose(tempfd);
    return 0;
}