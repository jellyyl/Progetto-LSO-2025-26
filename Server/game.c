#include "game.h"


//siccome usiamo extern dobbiamo inizializzare le variabili qui
int list_increment_game_id = 0;
pthread_mutex_t mutex_lista = PTHREAD_MUTEX_INITIALIZER;
Game * list_game[LIST_INIT_SIZE] = {NULL}; 


//utilizzare un arrays di gamesì globale per la lista delle partire

//Funzione principale che gestisce le azioni dell'utente
void game_action(int action, int client_id, int sd){
    switch (action)
    {
    case CREATE:
        create_game(client_id);
        break;
    case LIST:
        get_list_game(sd);
        break;
    case JOIN:
        join_game();
        break;
    case MOVE:
        move_character();
        break;
    case REMATCH:
        rematch();
        break;
    default:
        break;
    }
}

/*
Dovrebbe essere inutile se alloco dinamicamente l'array di partite con malloc
void init(){
    for (int i = 0; i<LIST_INIT_SIZE; i++){
        list_game[i] = malloc(sizeof(Game));
    }
}
*/


void get_list_game(int sd){
    char buffer[2048] = "";

    for (int i = 0; i<LIST_INIT_SIZE; i++){
        if (list_game[i] != NULL){
            char game_info[256];
            sprintf(game_info, " %d; %d; %d;\n", list_game[i]->id, list_game[i]->id_player1, list_game[i]->state);
            strcat(buffer, game_info);
        }
    }

    send(sd, buffer, strlen(buffer), 0);
}

//con allocazione dinamica dell'array sarebbe complessa perchè dovrei bloccare tutti i thread e quindi bloccare anche le partite
void create_game(int client_id){ 
   pthread_mutex_lock(&mutex_lista); 

   int found_index = -1;
    for (int i = 0; i < LIST_INIT_SIZE; i++) {
        if (list_game[i] == NULL) { //trovo il primo spazio libero
            found_index = i;
            break;
        }
    }

    if (found_index == -1) { //se ho la lista piena mando un errore
        pthread_mutex_unlock(&mutex_lista);
        return;
    }

   create_game_into_list(client_id, found_index);

   pthread_mutex_unlock(&mutex_lista);
}

void create_game_into_list(int client_id, int found_index){
   //inizializzazione game 
   list_game[found_index] = malloc(sizeof(Game));
   list_game[found_index] -> id = ++list_increment_game_id;
   list_game[found_index] -> id_player1 = client_id;
   list_game[found_index] -> id_player2 = -1; //non esiste ancora
   list_game[found_index] -> turn = 0;
   list_game[found_index] -> state = ST_WAITING;

   //inizializzazione tabellone 
   for(int r=0; r<3; r++){
        for(int c=0; c<3; c++){
            list_game[found_index]->table[r][c] = ' ';
        }
   }
   
   //inizializzazione del mutex
   pthread_mutex_init(&list_game[found_index]->game_mutex, NULL);
}




