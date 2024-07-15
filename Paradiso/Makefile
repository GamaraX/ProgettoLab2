# Compilatore
CC = gcc

# Flag di compilazione
CFLAGS = -Wall -g -pedantic -pthread

# Directory
HEADER = ../Purgatorio/
SORGENTE = ../Inferno/

# Header File
HDRS1 = $(HEADER)macro.h
HDRS2 = $(HEADER)Protocolli.h
HDRS3 = $(HEADER)Lista.h
HDRS4 = $(HEADER)Matrice.h
HDRS5 = $(HEADER)Dizionario.h
HDRS6 = $(HEADER)LogFun.h
HDRS7 = $(HEADER)Scorer.h

# Eseguibili
SERVER = Server
CLIENT = cliente
PROTO = Protocolli
LISTA = Lista
MATRICE = Matrice
DIZIO = Dizionario
LOGFUN = LogFun
SCORER = Scorer

# File oggetto
OBJS = $(SERVER).o
OBJC = $(CLIENT).o
OBJP = $(PROTO).o
OBJL = $(LISTA).o
OBJM = $(MATRICE).o
OBJD = $(DIZIO).o
OBJF = $(LOGFUN).o
OBJSC = $(SCORER).o

# File Sorgente
SRCS = $(SORGENTE)$(SERVER).c
SRCC = $(SORGENTE)$(CLIENT).c
SRCP = $(SORGENTE)$(PROTO).c
SRCL = $(SORGENTE)$(LISTA).c
SRCM = $(SORGENTE)$(MATRICE).c
SRCD = $(SORGENTE)$(DIZIO).c
SRCF = $(SORGENTE)$(LOGFUN).c
SRCSC = $(SORGENTE)$(SCORER).c

# Target
all: $(SERVER) $(CLIENT) objclear otherclean

# Dipendenza file eseguibile
$(SERVER): $(OBJS)
	@echo Server
	@$(CC) $(CFLAGS) $(OBJS) $(OBJP) $(OBJL) $(OBJM) $(OBJF) $(OBJD) $(OBJSC) -o $(SERVER)

$(CLIENT): $(OBJC)
	@echo Client
	@$(CC) $(CFLAGS) $(OBJC) $(OBJP) $(OBJL) $(OBJM) $(OBJD) -o $(CLIENT)

#forse OBJM non serve

# Dipendenza file oggetto

# Server.o
$(OBJS): $(SRCS) $(HDRS1) $(HDRS2) $(HDRS3) $(HDRS4) $(HDRS5) $(HDRS6) $(HDRS7)
	@$(CC) $(CFLAGS) -c $(SRCS) $(SRCL) $(SRCP) $(SRCF) $(SRCD) $(SRCM) $(SRCSC) $(HDRS1) $(HDRS2) $(HDRS3) $(HDRS4) $(HDRS5) $(HDRS6) $(HDRS7)
#Client.o
$(OBJC): $(SRCC) $(HDRS1) $(HDRS2) $(HDRS3) $(HDRS4)
	@$(CC) $(CFLAGS) -c $(SRCC) $(SRCL) $(SRCP) $(SRCM) $(HDRS1) $(HDRS2) $(HDRS3) $(HDRS4)
#Protocolli.o
$(OBJP): $(SRCP) $(HDRS1) $(HDRS2)
	@$(CC) $(CFLAGS) -c $(SRCP) $(HDRS1) $(HDRS2)
#Lista.o
$(OBJL): $(SRCL) $(HDRS1) $(HDRS3)
	@$(CC) $(CFLAGS) -c $(SRCL) $(HDRS1) $(HDRS3)
#Matrice.o
$(OBJM): $(SRCM) $(HDRS1) $(HDRS4)
	@$(CC) $(CFLAGS) -c $(SRCM) $(HDRS1) $(HDRS4)
#Dizionario.o
$(OBJD): $(SRCD) $(HDRS1) $(HDRS5)
	@$(CC) $(CFLAGS) -c $(SRCD) $(HDRS1) $(HDRS5)
#LogFun.o
$(OBJF): $(SRCF) $(HDRS1) $(HDRS6)
	@$(CC) $(CFLAGS) -c $(SRCF) $(HDRS1) $(HDRS6)
#Scorer.o
$(OBJSC): $(SRCSC) $(HDRS1) $(HDRS7)
	@$(CC) $(CFLAGS) -c $(SRCSC) $(HDRS1) $(HDRS7)
clean: 
	@rm -f $(OBJC) $(OBJS) $(OBJP) $(OBJL) $(OBJM) $(OBJD) $(SERVER) $(CLIENT)

objclear: 
	@rm -f $(OBJC) $(OBJS) $(OBJP) $(OBJL) $(OBJM) $(OBJF) $(OBJD)

otherclean:
	@rm -f $(HDRS1).gch $(HDRS2).gch $(HDRS3).gch $(HDRS4).gch $(HDRS5).gch $(HDRS6).gch

run:
	@./$(SERVER) 127.0.0.1 2000

run1:
	@./$(SERVER) 127.0.0.1 2004

run2:
	@./$(SERVER) 127.0.0.1 2001

client:
	@./$(CLIENT) 127.0.0.1 2000

client1:
	@./$(CLIENT) 127.0.0.1 2004

client2:
	@./$(CLIENT) 127.0.0.1 2001