
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "server.h"
#include "game.h"


// Funzione eseguita dal thread per gestire il client


int main() {

    int sd;
    sd = start_server(5200, 5);
    //init(); inutile con allocazione dinamica dell'array di partite
    while(1) {
        int newsd = accept_client(sd);

        handle_client(newsd, game_action());
    }


    close_server(sd);
    return 0;

}
