
//
typedef struct arg{
    int fd_client;
    pthread_t thread_id;
    Lista_Giocatori_Concorrente* lista;
} ThreadArgs;


//Funzione che permette al client di giocare
void* asdrubale (void* arg);