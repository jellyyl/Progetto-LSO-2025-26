CC = gcc
CFLAGS = -Wall -pthread
INCLUDES = -I./Server  # Dice a gcc di cercare gli header anche in Server/

# File Oggetto (il risultato della compilazione intermedia)
SERVER_OBJS = Server/main.o Server/server.o
CLIENT_OBJS = Client/clientTest.o

SERVER_BIN = server_exec
CLIENT_BIN = client_exec

all: $(SERVER_BIN) $(CLIENT_BIN)

# FASE DI LINKING: Unisce gli oggetti in un eseguibile
$(SERVER_BIN): $(SERVER_OBJS)
	$(CC) $(CFLAGS) $(SERVER_OBJS) -o $(SERVER_BIN)

$(CLIENT_BIN): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) $(CLIENT_OBJS) -o $(CLIENT_BIN)

# FASE DI COMPILAZIONE: Crea i file .o dai file .c
# % è un carattere jolly: trasforma ogni .c in un .o
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(SERVER_OBJS) $(CLIENT_OBJS) $(SERVER_BIN) $(CLIENT_BIN)

.PHONY: all clean