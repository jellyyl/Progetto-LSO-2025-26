# Variabili
CC = gcc
CFLAGS = -Wall -pthread

# Percorsi ai file sorgente
SERVER_SRC = Server/main.c
CLIENT_SRC = Client/clientTest.c

# Nomi degli ESEGUIBILI (in minuscolo per evitare conflitti con le cartelle)
SERVER_BIN = server_exec
CLIENT_BIN = client_exec

# Regola principale
all: $(SERVER_BIN) $(CLIENT_BIN)

# Compilazione del Server
$(SERVER_BIN): $(SERVER_SRC)
	$(CC) $(CFLAGS) $(SERVER_SRC) -o $(SERVER_BIN)

# Compilazione del Client
$(CLIENT_BIN): $(CLIENT_SRC)
	$(CC) $(CFLAGS) $(CLIENT_SRC) -o $(CLIENT_BIN)

# Regole per l'esecuzione
run-server: $(SERVER_BIN)
	./$(SERVER_BIN)

run-client: $(CLIENT_BIN)
	./$(CLIENT_BIN)

# Pulizia
clean:
	rm -f $(SERVER_BIN) $(CLIENT_BIN)

.PHONY: all clean run-server run-client