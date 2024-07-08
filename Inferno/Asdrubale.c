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



void* asdrubale (void* arg) {
    //Fd del client
    int fd_client = *(int*) arg;
    
    Giocatore giocatore;

    Msg* msg;

    while (1) {
        msg = Ade(fd_client);
        printf("%s", msg->msg);
        
        switch(msg->type){
            default:
                break;
        }

        
        free(msg->msg);
        free(msg->type);
    }


    /*while(1){
        int logged_in = 0;
        while(!logged_in){


            logged_in = 1;//sei riuscito a loggarti
        }
        int in_game = 1;
        //passi a gioco (sei loggato)        
        while(in_game) {
            

        }
    }*/
}
