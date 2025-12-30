
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



// Helper per creare partite al volo
Game create_mock_game(int id) {
    Game g;
    g.id = id;
    g.state = ST_NEW;
    pthread_mutex_init(&g.game_mutex, NULL);
    return g;
}


void test_game_vector_full() {
    printf("=== INIZIO TEST COMPLETI GAME_VECTOR ===\n");
    game_vector_t gv;
    init_game_vector(&gv);

    // --- TEST 1: ESPANSIONE DINAMICA (Resize Up) ---
    printf("[1] Test Espansione... ");
    // LIST_INIT_SIZE è 1, inseriamo 5 elementi per forzare diversi resize
    for(int i = 1; i <= 5; i++) {
        insert_game(&gv, create_mock_game(i * 10));
    }
    assert(gv.current_index == 5);
    assert(gv.size >= 5);
    printf("OK (Size: %d)\n", gv.size);

    // --- TEST 2: RICERCA PER ID ---
    printf("[2] Test Ricerca per ID... ");
    Game* p20 = find_game_by_id(&gv, 20);
    assert(p20 != NULL);
    assert(p20->id == 20);
    
    Game* pNULL = find_game_by_id(&gv, 999); // ID inesistente
    assert(pNULL == NULL);
    printf("OK\n");

    // --- TEST 3: RIMOZIONE CON SWAP (Il cuore dell'algoritmo) ---
    printf("[3] Test Rimozione con Swap... ");
    // Vettore attuale: [10, 20, 30, 40, 50] (indici 0,1,2,3,4)
    // Rimuoviamo l'ID 20 (indice 1). L'ID 50 (l'ultimo) deve finire all'indice 1.
    int res = remove_game_by_id(&gv, 20);
    assert(res == 0);
    assert(gv.current_index == 4);
    // Verifica dello swap:
    assert(gv.vector[1]->id == 50); 
    assert(gv.vector[4] == NULL); 
    printf("OK (ID 50 si è spostato all'indice 1)\n");

    // --- TEST 4: RIMOZIONE ULTIMO ELEMENTO ---
    printf("[4] Test Rimozione Ultimo... ");
    // Vettore attuale: [10, 50, 30, 40]
    res = remove_game_by_id(&gv, 40); 
    assert(res == 0);
    assert(gv.current_index == 3);
    assert(gv.vector[3] == NULL);
    printf("OK\n");

    // --- TEST 5: RIDUZIONE MEMORIA (Resize Down) ---
    printf("[5] Test Contrazione... ");
    // Rimuoviamo quasi tutto per vedere se il size scende
    remove_game(&gv, 0);
    remove_game(&gv, 0);
    // La logica di resize down dipende dalla tua soglia (es. count < size/4)
    printf("OK (Count attuale: %d, Size attuale: %d)\n", gv.count, gv.size);

    // --- TEST 6: PULIZIA FINALE ---
    printf("[6] Test Distruzione... ");
    destroy_game_vector(&gv);
    // Nota: dopo destroy, gv.vector dovrebbe essere stato liberato
    printf("OK\n");

    printf("=== TUTTI I TEST PASSATI CON SUCCESSO ===\n\n");
}

void test_resize_logic() {
printf("=== TEST RESIZE & SWAP (No Count) ===\n");
    game_vector_t gv;
    init_game_vector(&gv); // size = 1

    // 1. Test Espansione Automatica
    insert_game(&gv, create_mock_game(101)); // size diventa 2
    printf("[After 1 insert] current_index=%d, size=%d\n", gv.current_index, gv.size);
    insert_game(&gv, create_mock_game(102)); // size diventa 4
    printf("[After 2 inserts] current_index=%d, size=%d\n", gv.current_index, gv.size);
    insert_game(&gv, create_mock_game(103)); 
    printf("[After 3 inserts] current_index=%d, size=%d\n", gv.current_index, gv.size);
    insert_game(&gv, create_mock_game(104)); // size diventa 8
    printf("[After 4 inserts] current_index=%d, size=%d\n", gv.current_index, gv.size);
    
    printf("[1] Espansione: current_index=%d, size=%d\n", gv.current_index, gv.size);
    assert(gv.current_index == 4);
    assert(gv.size == 8);

    // 2. Test Swap with Last
    // Rimuoviamo l'id 101 (indice 0). Il 104 deve finire all'indice 0.
    remove_game(&gv, 0); 
    assert(gv.vector[0]->id == 104);
    assert(gv.current_index == 3);
    printf("[2] Swap OK: ID 104 spostato in indice 0\n");

    // 3. Test Contrazione al 25%
    // Attualmente: index=3, size=8. Rimuoviamo finché index <= 2 (8/4=2)
    remove_game(&gv, 0); // index=2. Qui scatta il resize(8/2 = 4)
    
    printf("[3] Contrazione: current_index=%d, size=%d\n", gv.current_index, gv.size);
    assert(gv.size == 4);
    assert(gv.current_index == 2);

    destroy_game_vector(&gv);
    printf("=== TEST COMPLETATI CON SUCCESSO ===\n\n");
}

int main() {
    // Esegui i test di game_vector
    //test_game_vector_full();
    test_resize_logic();
    
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
