#include "game_vector.h"


void init_game_vector(game_vector_t* game_v){
    game_v->size = LIST_INIT_SIZE;
    game_v->vector = malloc(sizeof(Game*) * game_v->size);
    game_v->current_index = 0;
    pthread_mutex_init(&game_v->mutex_list, NULL);
    for (int i = 0; i<game_v->size; i++){
        game_v->vector[i] = NULL;
    }
}

void destroy_game_vector(game_vector_t* game_v){

    pthread_mutex_destroy(&game_v->mutex_list);

    free_vector(game_v->vector, game_v->size);
}


void insert_game_into_vector(game_vector_t* game_v, Game* new_game){
    pthread_mutex_lock(&game_v->mutex_list);

    int index = game_v->current_index;

    if (index+1 >= game_v->size){
        resize(game_v, game_v->size*2);
    }

    Game* temp_game = malloc(sizeof(Game));
    *temp_game = *new_game;
    game_v->vector[index] = temp_game;

    //incremento indice corrente
    game_v->current_index++;

    pthread_mutex_unlock(&game_v->mutex_list);
}

Game* get_game_by_id(game_vector_t* game_v, int game_id){

    Game* found_game = NULL;

    pthread_mutex_lock(&game_v->mutex_list);
    
    int found_index = find_index_by_game_id(game_v, game_id);

    if(found_index != -1){
        found_game = game_v->vector[found_index];
    }

    pthread_mutex_unlock(&game_v->mutex_list);
    
    return found_game;
}

int find_index_by_game_id(game_vector_t* game_v, int game_id){

    int found_index = -1;
    int game_found = 0;

    for(int i=0; i<game_v->current_index && !game_found; i++){
        if (game_v->vector[i] != NULL && game_v->vector[i]->id == game_id){
            found_index = i;
            game_found = 1;
        }
    }

    return found_index;
}

int remove_game_by_id(game_vector_t* game_v, int game_id){

    int result = -1;

    pthread_mutex_lock(&game_v->mutex_list);

    int found_index = find_index_by_game_id(game_v, game_id);

    if(found_index != -1){
        result = remove_game(game_v, found_index);
    }
    
    pthread_mutex_unlock(&game_v->mutex_list);

    return result;
}

//private helper function
int remove_game(game_vector_t* game_v, int index){


    int result = -1;

    if(index < game_v->size && game_v->vector[index]!=NULL) {

        int current_index = game_v->current_index;
        Game* ptr = game_v->vector[index];
        game_v->vector[index] = game_v->vector[current_index-1];
        game_v->vector[current_index-1] = NULL;
        free(ptr);
        game_v->current_index--;
        result=0;

        if(game_v->current_index <= (game_v->size/4)) {
            resize(game_v, game_v->size/2);
        }
            
    }

    return result;
}

int resize(game_vector_t* game_v, int new_size) {

    if(!game_v || !game_v->vector || new_size <= 0) {
        return -1;
    }

    int old_size = game_v->size;

    if(new_size < game_v->current_index) {
        free_vector_interval(game_v->vector, new_size, game_v->current_index);
        game_v->current_index = new_size;
    }

    Game** temp_vector = realloc(game_v->vector, sizeof(Game*) * new_size);

    if(!temp_vector) {
        return -1;
    }

    game_v->vector = temp_vector;
    game_v->size = new_size;

    if(new_size > old_size) {

        for (int i = old_size; i<new_size; i++){
            game_v->vector[i] = NULL;
        }
    }

    printf("current index: %d", game_v->current_index);    
    return 0;
    
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
            vector[i] = NULL;
        }
    }
}