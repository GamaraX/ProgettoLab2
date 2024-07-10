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
HDRS3 = $(HEADER)ListaClient.h
HDRS4 = $(HEADER)Matrice.h
HDRS6 = $(HEADER)LogFun.h

# Eseguibili
SERVER = Server
CLIENT = cliente
PROTO = Protocolli
LISTA = ListaClient
MATRICE = Matrice
LOGFUN = LogFun

# File oggetto
OBJS = $(SERVER).o
OBJC = $(CLIENT).o
OBJP = $(PROTO).o
OBJL = $(LISTA).o
OBJM = $(MATRICE).o
OBJF = $(LOGFUN).o

# File Sorgente
SRCS = $(SORGENTE)$(SERVER).c
SRCC = $(SORGENTE)$(CLIENT).c
SRCP = $(SORGENTE)$(PROTO).c
SRCL = $(SORGENTE)$(LISTA).c
SRCM = $(SORGENTE)$(MATRICE).c
SRCF = $(SORGENTE)$(LOGFUN).c

# Target
all: $(SERVER) $(CLIENT) objclear otherclean

# Dipendenza file eseguibile
$(SERVER): $(OBJS)
	@echo Server
	@$(CC) $(CFLAGS) $(OBJS) $(OBJP) $(OBJL) $(OBJM) $(OBJF) -o $(SERVER)

$(CLIENT): $(OBJC)
	@echo Client
	@$(CC) $(CFLAGS) $(OBJC) $(OBJP) $(OBJL) $(OBJM) -o $(CLIENT)

#forse OBJM non serve

# Dipendenza file oggetto

# Server.o
$(OBJS): $(SRCS) $(HDRS1) $(HDRS2) $(HDRS3) $(HDRS4) $(HDRS6)
	@$(CC) $(CFLAGS) -c $(SRCS) $(SRCL) $(SRCP) $(SRCF) $(SRCM) $(HDRS1) $(HDRS2) $(HDRS3) $(HDRS4) $(HDRS6)
#Client.o
$(OBJC): $(SRCC) $(HDRS1) $(HDRS2) $(HDRS3) $(HDRS4)
	@$(CC) $(CFLAGS) -c $(SRCC) $(SRCL) $(SRCP) $(SRCM) $(HDRS1) $(HDRS2) $(HDRS3) $(HDRS4)
#Protocolli.o
$(OBJP): $(SRCP) $(HDRS1) $(HDRS2)
	@$(CC) $(CFLAGS) -c $(SRCP) $(HDRS1) $(HDRS2)
#ListaClient.o
$(OBJL): $(SRCL) $(HDRS1) $(HDRS3)
	@$(CC) $(CFLAGS) -c $(SRCL) $(HDRS1) $(HDRS3)
#Matrice.o
$(OBJM): $(SRCM) $(HDRS1) $(HDRS4)
	@$(CC) $(CFLAGS) -c $(SRCM) $(HDRS1) $(HDRS4)

#LogFun.o
$(OBJF): $(SRCF) $(HDRS1) $(HDRS6)
	@$(CC) $(CFLAGS) -c $(SRCF) $(HDRS1) $(HDRS6)

clean: 
	@rm -f $(OBJC) $(OBJS) $(OBJP) $(OBJL) $(OBJM) $(SERVER) $(CLIENT)

objclear: 
	@rm -f $(OBJC) $(OBJS) $(OBJP) $(OBJL) $(OBJM) $(OBJF)

otherclean:
	@rm -f $(HDRS1).gch $(HDRS2).gch $(HDRS3).gch $(HDRS4).gch $(HDRS6).gch

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