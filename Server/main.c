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

// Helper per creare partite al volo per i test
Game create_mock_game(int id) {
    Game g;
    g.id = id;
    g.state = ST_NEW;
    // Inizializziamo il mutex della singola partita
    pthread_mutex_init(&g.game_mutex, NULL);
    return g;
}
/*
void test_game_vector_full() {
    printf("=== INIZIO TEST COMPLETI GAME_VECTOR ===\n");
    game_vector_t gv;
    init_game_vector(&gv); // Inizializza con size = LIST_INIT_SIZE

    // --- TEST 1: ESPANSIONE DINAMICA (Resize Up) ---
    printf("[1] Test Espansione... ");
    for(int i = 1; i <= 5; i++) {
        // Usiamo una variabile locale per passare l'indirizzo a insert_game
        Game temp = create_mock_game(i * 10);
        insert_game(&gv, &temp); 
    }
    // Verifichiamo current_index invece di count
    assert(gv.current_index == 5);
    assert(gv.size >= 5);
    printf("OK (Size: %d)\n", gv.size);

    // --- TEST 2: RICERCA PER ID ---
    printf("[2] Test Ricerca per ID... ");
    Game* p20 = get_game_by_id(&gv, 20);
    assert(p20 != NULL);
    assert(p20->id == 20);
    
    Game* pNULL = get_game_by_id(&gv, 999); 
    assert(pNULL == NULL);
    printf("OK\n");

    // --- TEST 3: RIMOZIONE CON SWAP (Il cuore dell'algoritmo) ---
    printf("[3] Test Rimozione con Swap... ");
    // Vettore attuale: [10, 20, 30, 40, 50] (indici 0,1,2,3,4)
    // Rimuoviamo l'ID 20 (indice 1). L'ID 50 (l'ultimo) deve finire all'indice 1.
    int res = remove_game_by_id(&gv, 20);
    assert(res == 0);
    assert(gv.current_index == 4);
    
    // Verifica dello swap: l'ultimo elemento (50) ha preso il posto del rimosso (20)
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
    // Rimuoviamo elementi per testare il ridimensionamento automatico
    remove_game(&gv, 0);
    remove_game(&gv, 0);
    printf("OK (current_index attuale: %d, Size attuale: %d)\n", gv.current_index, gv.size);

    // --- TEST 6: PULIZIA FINALE ---
    printf("[6] Test Distruzione... ");
    destroy_game_vector(&gv);
    printf("OK\n");

    printf("=== TUTTI I TEST PASSATI CON SUCCESSO ===\n\n");
}
*/
/*
void test_resize_logic() {
    printf("=== TEST RESIZE & SWAP (No Count) ===\n");
    game_vector_t gv;
    init_game_vector(&gv); // Inizializza con size = 1

    // 1. Test Espansione Automatica
    // Usiamo variabili temporanee per evitare l'errore "lvalue required"
    Game g1 = create_mock_game(101);
    insert_game_into_vector(&gv, &g1); // size diventa 2
    printf("[After 1 insert] current_index=%d, size=%d\n", gv.current_index, gv.size);
    
    Game g2 = create_mock_game(102);
    insert_game_into_vector(&gv, &g2); // size diventa 4
    printf("[After 2 inserts] current_index=%d, size=%d\n", gv.current_index, gv.size);
    
    Game g3 = create_mock_game(103);
    insert_game_into_vector(&gv, &g3); 
    printf("[After 3 inserts] current_index=%d, size=%d\n", gv.current_index, gv.size);
    
    Game g4 = create_mock_game(104);
    insert_game_into_vector(&gv, &g4); // size diventa 8
    printf("[After 4 inserts] current_index=%d, size=%d\n", gv.current_index, gv.size);
    
    assert(gv.current_index == 4);
    assert(gv.size == 8);

    // 2. Test Swap with Last
    // Rimuoviamo l'id 101 (indice 0). L'id 104 deve finire all'indice 0.
    remove_game(&gv, 0); 
    assert(gv.vector[0]->id == 104);
    assert(gv.current_index == 3);
    printf("[2] Swap OK: ID 104 spostato in indice 0\n");

    // 3. Test Contrazione al 25% (Soglia size/4)
    // Se la soglia nel tuo game_vector.c è current_index <= size/4
    // Attualmente: index=3, size=8. Rimuovendo uno (index=2), 2 <= 8/4? Sì.
    remove_game(&gv, 0); 
    
    printf("[3] Contrazione: current_index=%d, size=%d\n", gv.current_index, gv.size);
    assert(gv.size == 4);
    assert(gv.current_index == 2);

    destroy_game_vector(&gv);
    printf("=== TEST RESIZE COMPLETATI CON SUCCESSO ===\n\n");
}
*/

int main() {
    
    //test_game_vector_full();
    //test_resize_logic();

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