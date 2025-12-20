#ifndef GAME_H
#define GAME_H

#include <pthread.h>

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
    pthread_mutex_t game_mute;
} Game;

void game_action(int action, int client_id);

#endif