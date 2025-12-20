#ifndef GAME_H
#define GAME_H

#include <pthread.h>

//Mutex per proteggere la ricezione lista delle partite
pthread_mutex_t mutex_lista = PTHREAD_MUTEX_INITIALIZER;

const int list_init_size = 10;
int list_increment_size = 0;

// Array di partite disponibile per tutti i thread
Game * list_game[list_init_size];

//Azioni del giocatore
typedef enum{
    CREATE = 1,
    LIST = 2,
    JOIN = 3,
    MOVE = 4,
    REMATCH = 5
} Actions; 

typedef enum {
    ST_NEW,      
    ST_WAITING,  
    ST_PLAYING,  
    ST_FINISHED  
} GameState;

typedef struct{
    int id;
    int id_player1;
    int id_player2;
    char table[3][3]; //così char posso mettere visivamente "X" e "O"
    int state;
    int turn;
    pthread_mutex_t game_mutex;
} Game;

void game_action(int action, int client_id, int sd);
void get_list_game();
void init();

#endif