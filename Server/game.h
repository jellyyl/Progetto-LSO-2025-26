#ifndef GAME_H
#define GAME_H

#include <pthread.h>

// forward declaration per evitare include circolare
typedef struct game_vector_t game_vector_t;

//Azioni del giocatore
typedef enum{
    CREATE = 1,
    LIST = 2,
    JOIN = 3,
    MOVE = 4,
    REMATCH = 5,
    APPROVE = 6
} Actions; 

typedef enum {
    ST_NEW,      
    ST_WAITING, 
    ST_APPROVE,
    ST_PLAYING,  
    ST_FINISHED  
} GameState;

typedef struct Game{
    int id;
    int id_player1;
    int id_player2;
    char table[3][3]; //così char posso mettere visivamente "X" e "O"
    int state;
    int turn; //0 player 1; 1 player 2;
    pthread_mutex_t game_mutex;
    pthread_cond_t cond_approve;
} Game;

extern game_vector_t game_vector;

void game_action(void *arg);
void get_list_game(int sd);
void create_game(int client_id);
Game generate_game(int client_id);
void join_game(int client_id, int game_id, int sd);
void approve_join_request(int game_id, int sd, int response);

#endif