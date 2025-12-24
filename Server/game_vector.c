#include "game_vector.h"



void init_game_vector(game_vector_t* game_v){
    game_v->size = LIST_INIT_SIZE;
    game_v->vector = malloc(sizeof(Game*) * game_v->size);
    game_v->current_index = 0;
    game_v->count = 0;
    pthread_mutex_init(&game_v->mutex_list, NULL);
    for (int i = 0; i<game_v->size; i++){
        game_v->vector[i] = NULL;
    }
}

void destroy_game_vector(game_vector_t* game_v){

    pthread_mutex_destroy(&game_v->mutex_list);

    free_vector(game_v->vector, game_v->size);
}


void insert_game(game_vector_t* game_v, Game new_game){
    pthread_mutex_lock(&game_v->mutex_list);

    int index = game_v->current_index;

    if (index+1 >= game_v->size){
        resize(game_v, game_v->size*2);
    }

    game_v->vector[index] = malloc(sizeof(Game));
    *(game_v->vector[index]) = new_game;
    
    //incremento indice corrente
    game_v->current_index++;

    //incremento contatore partite
    game_v->count++;


    pthread_mutex_unlock(&game_v->mutex_list);
}

Game* get_game(game_vector_t* game_v, int index){

    Game* out_game = NULL;

    pthread_mutex_lock(&game_v->mutex_list);

    if (index >= 0 && index < game_v->size){
        out_game = game_v->vector[index];
    }

    pthread_mutex_unlock(&game_v->mutex_list);

    return out_game;
}


int remove_game(game_vector_t* game_v, int index){


    int result = -1;
    pthread_mutex_lock(&game_v->mutex_list);

    if(index < game_v->size && game_v->vector[index]!=NULL) {

        Game* ptr = game_v->vector[index];
        game_v->vector[index] = NULL;
        free(ptr);
        game_v->count--;
        result=0;

        if(game_v->count < (game_v->size/2)) {
            resize(game_v, game_v->size/2);
        }
            
        
    }

    pthread_mutex_unlock(&game_v->mutex_list);

    return result;
}

int resize(game_vector_t* game_v, int new_size) {

    if(game_v->vector !=NULL){

        Game** temp_vector = malloc(sizeof(Game*) * new_size);
        int effective_size;
        int size = game_v->size;


        if(size < new_size) {
            effective_size = size;
        } else {
            effective_size = new_size;
        }

        int temp_curr_index=0;

        for(int i=0; i<effective_size; i++) {

            if(game_v->vector[i] != NULL) {
                Game* g_copy = malloc(sizeof(Game));
                Game 
                *(g_copy) = 
                temp_vector[temp_curr_index];
                game_v->vector[i];
                temp_curr_index++;
            }
        }
        
        free(game_v->vector);
        game_v->vector = temp_vector;
        game_v->current_index = temp_curr_index;
        game_v->count = temp_curr_index;
        game_v->size = new_size;
        return 0;
    }

    return -1;

}

void free_vector(Game** vector, int size){

    for(int i=0; i<size; i++){
        if (vector[i] != NULL){
            free(vector[i]);
        }
    }
    free(vector);
}

void move_vector(game_vector_t* game_v1, game_vector_t* game_v2) {


    int size1 = game_v1->size;
    int size2 = game_v2->size;
    Game** vector1 = game_v1->vector;
    Game** vector2 = game_v2->vector;;

    int effective_size = game_v1->size;
    int temp_curr_index=0;

    if(size1 < size2) {
        effective_size = size1;
    } 

    for(int i=0; i<effective_size; i++) {

        if(vector2[i] != NULL) {
            Game* g_copy = malloc(sizeof(Game));
            Game game_elem = *(vector2[i]);
            *(g_copy) = game_elem;
            vector1[temp_curr_index] = g_copy;
            temp_curr_index++;
        }
    }

    free_vector(game_v2->vector, game_v2->size);

    game_v1->current_index = temp_curr_index;
    game_v1->count= game_v2->count;
    game_v1->mutex_list = game_v2->mutex_list;

    

}