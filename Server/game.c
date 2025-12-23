#include "game.h"

//utilizzare un arrays di gamesì globale per la lista delle partire

//Funzione principale che gestisce le azioni dell'utente
void game_action(int action, int client_id, int sd){

    while(1) {
        switch (action)
        {
            case CREATE:
                create_game();
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

}

void init(){
    for (int i = 0; i<list_increment_size; i++){
        list_game[i] = malloc(sizeof(Game));
    }
}

void get_list_game(int sd){
    char buffer[1024] = "";

    for (int i = 0; i<list_increment_size; i++){
        char game_info[256];
        sprintf(game_info, " %d; %d; %d;\n", list_game[i]->id, list_game[i]->id_player1, list_game[i]->state);
        strcat(buffer, game_info);
    }

    send(sd, buffer, strlen(buffer), 0);
}





