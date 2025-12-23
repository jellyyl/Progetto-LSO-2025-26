#ifndef GAME_H
#define GAME_H

#define LIST_INIT_SIZE 100

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
    ST_APPROVE,
    ST_PLAYING,  
    ST_FINISHED  
} GameState;

typedef struct{
    int id;
    int id_player1;
    int id_player2;
    char table[3][3]; //così char posso mettere visivamente "X" e "O"
    int state;
    int turn; //0 player 1; 1 player 2;
    pthread_mutex_t game_mutex;
    pthread_cond_t cond_approve;
} Game;

//exern serve per rendere visibili le variabili in altri file, se non ci fosse ogni file avrebbe la sua copia
//Mutex per proteggere la ricezione lista delle partite
extern pthread_mutex_t mutex_lista;
extern int list_increment_game_id;

// Array di partite disponibile per tutti i thread
extern Game * list_game[LIST_INIT_SIZE];

void game_action(int action, int client_id, int sd);
void get_list_game(int sd);
void create_game(int client_id);
void create_game_into_list(int client_id, int found_index);
void join_game(int client_id, int game_id, int sd);

#endif