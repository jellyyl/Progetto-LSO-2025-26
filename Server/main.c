#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "game.h"
#include "game_vector.h"
#include "server.h"

int main() {
    
    int port = 5200;
    char *env_port = getenv("PORT");
    if (env_port != NULL) {
        port = atoi(env_port); //converte stringa in intero
    }
    
    int sd;
    sd = start_server(port, 5);
    init_game_session();

    while(1) {
        int newsd = accept_client(sd);
        handle_client(newsd, game_action);
    }

    close_game_session();
    close_server(sd);
    
    return 0;
}