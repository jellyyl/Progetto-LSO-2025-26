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

    if (index >= 0 && index < game_v->size){
        out_game = game_v->vector[index];
    }

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

        Game** temp_vector = calloc(new_size, sizeof(Game*));
        int size = game_v->size;
        int effective_size = size;
        int temp_curr_index=0;

        if(size > new_size) {
            effective_size = new_size;
        } 

        for(int i=0; i<effective_size; i++) {

            if(game_v->vector[i] != NULL) {
                temp_vector[temp_curr_index] = game_v->vector[i];
                temp_curr_index++;
            }
        }
        
        if(size > new_size) {
            //libero gli elementi non copiati
            free_vector_interval(game_v->vector, temp_curr_index, size);
        }

        free(game_v->vector);
        game_v->vector = temp_vector;
        game_v->current_index = temp_curr_index;
        game_v->count = temp_curr_index;
        game_v->size = new_size;

        printf("current index: %d", game_v->current_index);
        printf("Resized game vector from size: %d to size: %d\n", size, new_size);
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

void free_vector_interval(Game** vector, int start, int end){

    for(int i=start; i<end; i++){
        if (vector[i] != NULL){
            free(vector[i]);
        }
    }
}