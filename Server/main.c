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
    
    int sd;
    sd = start_server(5200, 5);
    init_game_session();

    while(1) {
        int newsd = accept_client(sd);
        handle_client(newsd, game_action);
    }

    close_game_session();
    close_server(sd);
    
    return 0;
}