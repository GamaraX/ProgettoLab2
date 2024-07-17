#define SYSC(v,c,m)\
    if ((v = c ) == -1){perror(m);exit(errno);};

#define SYSCN(v,c,m)\
    if ((v = c) == NULL){perror(m);exit(errno);};

#define SYST(v,c,m)\
    if ((v = c) != 0){perror(m);exit(errno);};

//Funzione che non serve a nulla se non a prevenire un errore da parte del makefile
void fun();
