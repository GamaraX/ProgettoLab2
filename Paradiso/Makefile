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

# Eseguibili
SERVER = Server
CLIENT = cliente
PROTO = Protocolli

# File oggetto
OBJS = $(SERVER).o
OBJC = $(CLIENT).o
OBJP = $(PROTO).o

# File Sorgente
SRCS = $(SORGENTE)$(SERVER).c
SRCC = $(SORGENTE)$(CLIENT).c
SRCP = $(SORGENTE)$(PROTO).c

# Target
all: $(SERVER) $(CLIENT) objclear otherclean

# Dipendenza file eseguibile
$(SERVER): $(OBJS) 
	@echo Server
	@$(CC) $(CFLAGS) $(OBJS) $(OBJP) -o $(SERVER)

$(CLIENT): $(OBJC) 
	@echo Client
	@$(CC) $(CFLAGS) $(OBJC) $(OBJP) -o $(CLIENT)

# Dipendenza file oggetto

# Server.o
$(OBJS): $(SRCS) $(HDRS1) $(HDRS2) 
	@$(CC) $(CFLAGS) -c $(SRCS) $(SRCP) $(HDRS1) $(HDRS2)

$(OBJC): $(SRCC) $(HDRS1) $(HDRS2) 
	@$(CC) $(CFLAGS) -c $(SRCC) $(SRCP) $(HDRS1) $(HDRS2)

$(OBJP): $(SRCP) $(HDRS1) $(HDRS2) 
	@$(CC) $(CFLAGS) -c $(SRCP) $(HDRS1) $(HDRS2)

clean: 
	@rm -f $(OBJC) $(OBJS) $(OBJP) $(SERVER) $(CLIENT)

objclear: 
	@rm -f $(OBJC) $(OBJS) $(OBJP)

otherclean:
	@rm -f $(HDRS1).gch $(HDRS2).gch