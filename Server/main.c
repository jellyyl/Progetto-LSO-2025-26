
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include "server.h"
#include "game.h"
#include "game_vector.h"
#include <assert.h>




// Funzione helper per creare un oggetto Game con un ID specifico
Game create_mock_game(int id) {
    Game g;
    g.id = id;
    g.id_player1 = 1000 + id;
    g.id_player2 = 2000 + id;
    g.state = ST_NEW;
    g.turn = 0;
    // Inizializziamo la scacchiera vuota
    memset(g.table, ' ', sizeof(g.table));
    pthread_mutex_init(&g.game_mutex, NULL);
    return g;
}

void test_game_vector_logic() {
    printf("=== INIZIO TEST CONCRETO GAME_VECTOR ===\n");

    game_vector_t gv;
    
    // --- TEST 1: INIZIALIZZAZIONE ---
    printf("[1] Test Inizializzazione... ");
    init_game_vector(&gv);
    assert(gv.size == LIST_INIT_SIZE);
    assert(gv.count == 0);
    assert(gv.current_index == 0);
    printf("OK\n");

    // --- TEST 2: INSERIMENTO E PROGRESSIONE INDICI ---
    printf("[2] Test Inserimento... ");
    insert_game(&gv, create_mock_game(10)); // Indice 0
    insert_game(&gv, create_mock_game(20)); // Indice 1
    insert_game(&gv, create_mock_game(30)); // Indice 2

    assert(gv.count == 3);
    assert(gv.current_index == 3);
    remove_game(&gv, 1); // Rimuovi gioco all'indice 1
    assert(gv.count == 2);
    assert(gv.current_index == 3); // current_index non cambia
    int code = remove_game(&gv, 1); // Rimuovi gioco all'indice 1
    assert(code == -1);
    assert(gv.current_index == 3); // current_index non cambia
    printf("OK\n");
    printf("%d\n", gv.vector[0]->id);
    printf("%d\n", gv.vector[2]->id);
    assert(gv.count == 2);
    code = remove_game(&gv, 0); // Rimuovi gioco all'indice 0
    assert(code == 0);
    printf("%d\n", gv.count);
    assert(gv.count == 1);
    assert(gv.current_index == 1); // current_index non cambia

}

int main() {
    // Esegui i test di game_vector
    test_game_vector_logic();
    
    // Decommentare le righe seguenti quando si vuole avviare il server vero
    /*
    int sd;
    sd = start_server(5200, 5);
    while(1) {
        int newsd = accept_client(sd);
        handle_client(newsd, game_action());
    }
    close_server(sd);
    */
    
    return 0;
}
