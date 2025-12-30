#ifndef GAME_VECTOR_H
#define GAME_VECTOR_H

#include <pthread.h>
#include "game.h"

#define LIST_INIT_SIZE 1

//valutare di dichiarare la struct game qui dentro game_list.h

typedef struct game_vector_t{
    Game ** vector;
    pthread_mutex_t mutex_list;
    int current_index;
    int size;
} game_vector_t;


void init_game_vector(game_vector_t* game_v);
void destroy_game_vector(game_vector_t* game_v);
void insert_game(game_vector_t* game_v, Game new_game);
int find_index_by_game_id(game_vector_t* game_v, int game_id);
Game* get_game(game_vector_t* game_v, int index);
Game* get_game_by_id(game_vector_t* game_v, int game_id);
int remove_game_by_id(game_vector_t* game_v, int game_id);
int remove_game(game_vector_t* game_v, int index);
int resize(game_vector_t* game_v, int new_size);


#endif