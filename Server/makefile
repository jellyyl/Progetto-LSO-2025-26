CC = gcc
CFLAGS = -Wall -Wextra -pthread -g

# File Oggetto per il server
SERVER_OBJS = main.o game_vector.o game.o server.o
SERVER_BIN = server_exec

all: $(SERVER_BIN)

# FASE DI LINKING: Unisce gli oggetti in un eseguibile
$(SERVER_BIN): $(SERVER_OBJS)
	$(CC) $(CFLAGS) $(SERVER_OBJS) -o $(SERVER_BIN)

# FASE DI COMPILAZIONE: Crea i file .o dai file .c
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SERVER_OBJS) $(SERVER_BIN)

run: $(SERVER_BIN)
	./$(SERVER_BIN)

.PHONY: all clean run
