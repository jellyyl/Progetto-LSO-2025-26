#ifndef GAME_VECTOR_H
#define GAME_VECTOR_H

#include <pthread.h>
#include "game.h"

#define LIST_INIT_SIZE 100

//valutare di dichiarare la struct game qui dentro game_list.h

typedef struct game_vector_t{
    Game ** vector;
    pthread_mutex_t mutex_list;
    int current_index;
    int size;
    int count;
} game_vector_t;


void init_game_vector(game_vector_t* game_v);
void destroy_game_vector(game_vector_t* game_v);
void insert_game(game_vector_t* game_v, Game new_game);
Game* get_game(game_vector_t* game_v, int index);
int remove_game(game_vector_t* game_v, int index);
int resize(game_vector_t* game_v, int new_size);


#endif