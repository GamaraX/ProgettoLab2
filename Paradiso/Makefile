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

# Eseguibili
SERVER = Server
CLIENT = cliente
PROTO = Protocolli
LISTA = ListaClient
MATRICE = Matrice

# File oggetto
OBJS = $(SERVER).o
OBJC = $(CLIENT).o
OBJP = $(PROTO).o
OBJL = $(LISTA).o
OBJM = $(MATRICE).o

# File Sorgente
SRCS = $(SORGENTE)$(SERVER).c
SRCC = $(SORGENTE)$(CLIENT).c
SRCP = $(SORGENTE)$(PROTO).c
SRCL = $(SORGENTE)$(LISTA).c
SRCM = $(SORGENTE)$(MATRICE).c

# Target
all: $(SERVER) $(CLIENT) objclear otherclean

# Dipendenza file eseguibile
$(SERVER): $(OBJS) 
	@echo Server
	@$(CC) $(CFLAGS) $(OBJS) $(OBJP) $(OBJL) -o $(SERVER)

$(CLIENT): $(OBJC) 
	@echo Client
	@$(CC) $(CFLAGS) $(OBJC) $(OBJP) $(OBJL) -o $(CLIENT)

# Dipendenza file oggetto

# Server.o
$(OBJS): $(SRCS) $(HDRS1) $(HDRS2) $(HDRS3)
	@$(CC) $(CFLAGS) -c $(SRCS) $(SRCL) $(SRCP) $(HDRS1) $(HDRS2) $(HDRS3)

$(OBJC): $(SRCC) $(HDRS1) $(HDRS2) $(HDRS3)
	@$(CC) $(CFLAGS) -c $(SRCC) $(SRCL) $(SRCP) $(HDRS1) $(HDRS2) $(HDRS3)

$(OBJP): $(SRCP) $(HDRS1) $(HDRS2)
	@$(CC) $(CFLAGS) -c $(SRCP) $(HDRS1) $(HDRS2)

$(OBJL): $(SRCL) $(HDRS1) $(HDRS3)
	@$(CC) $(CFLAGS) -c $(SRCL) $(HDRS1) $(HDRS3)

$(OBJM): $(SRCM) $(HDRS1) $(HDRS4)
	@$(CC) $(CFLAGS) -c $(SRCM) $(HDRS1) $(HDRS4)

clean: 
	@rm -f $(OBJC) $(OBJS) $(OBJP) $(OBJL) $(OBJM) $(SERVER) $(CLIENT)

objclear: 
	@rm -f $(OBJC) $(OBJS) $(OBJP) $(OBJL) $(OBJM)

otherclean:
	@rm -f $(HDRS1).gch $(HDRS2).gch $(HDRS3).gch $(HDRS4).gch

run:
	@./$(SERVER) 127.0.0.1 2000

run2:
	@./$(SERVER) 127.0.0.1 2001

client:
	@./$(CLIENT) 127.0.0.1 2000

client2:
	@./$(CLIENT) 127.0.0.1 2001