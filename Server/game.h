#ifndef GAME_H
#define GAME_H

#include <pthread.h>

// forward declaration per evitare include circolare
typedef struct game_vector_t game_vector_t;


//Azioni del giocatore
typedef enum{
    CREATE,
    LIST,
    JOIN,
    MOVE,
    REMATCH,
    APPROVE,
    CANCEL
} Actions; 

typedef enum {
    ST_NEW,      
    ST_WAITING, 
    ST_APPROVE,
    ST_PLAYING,  
    ST_FINISHED  
} GameState;

typedef enum {
    // --- JOIN PHASE (200-299) ---
    MSG_JOIN_OK = 200,
    MSG_JOIN_REQUEST = 201,     // Quando notifichi P1 che qualcuno vuole entrare
    MSG_JOIN_DENIED = 202,      // Partita piena o rifiutata

    // --- GAME PHASE (300-399) ---
    MSG_START_PLAYER1 = 300,
    MSG_START_PLAYER2 = 301,
    MSG_CANCELLED = 302,        // Quando P1 rifiuta la richiesta
    MSG_CHANGE_OWNER = 303,     // Quando P2 vince e diventa host

    // --- REMATCH PHASE (400-499) ---
    MSG_REMATCH_REQUEST = 400,
    MSG_REMATCH_DECLINED = 401,

    // --- ERRORS (500+) ---
    ERR_JOIN_NOT_FOUND = 500,
    ERR_JOIN_OWNER_LEFT = 501,
    ERR_APPROVE_NOT_FOUND = 502,
    ERR_REMATCH_NOT_FOUND = 503,
    ERR_REMATCH_NOT_PLAYER = 504,
    ERR_REMATCH_NOT_FINISHED = 505,
    ERR_REMATCH_NOT_OWNER = 506


} ResponseCode;

typedef enum {
    CMD_UNKNOWN = 0, 
    CMD_PLAY,        
    CMD_WAIT,        
    CMD_OVER,        
    CMD_WIN,  //4       
    CMD_DRAW,    //5    
    CMD_LOSE,       //6
    CMD_INVALID,     
    CMD_QUIT         
} GameCommand;


typedef struct Game{
    int id;
    int id_player1;
    int id_player2;
    char table[3][3]; //così char posso mettere visivamente "X" e "O"
    GameState state;
    int turn; //0 player 1; 1 player 2;
    pthread_mutex_t game_mutex;
    pthread_cond_t cond_approve;
    int rematch_status_player1; // 0 = no, 1 = yes
    int rematch_status_player2; // 0 = no, 1 = yes
} Game;

extern game_vector_t game_vector;

void *game_action(void *arg);
void get_list_game(int sd);
int create_game(int client_id);
Game generate_game(int client_id);
void join_game(int client_id, int game_id, int sd);
void approve_join_request(int game_id, int sd, int response);
void init_game_session();
void close_game_session();
int game_over(Game* game, int winner_sd);
int change_owner_game(Game* game);
int ask_rematch(Game* game, int winner_sd);
int rematch_from_both(Game* game, int sd, int response);
int rematch_by_winner(Game* game, int sd, int response);
int do_rematch(int game_id, int sd);
int clear_game(Game *game);



#endif