#include "game.h"

//utilizzare un arrays di gamesì globale per la lista delle partire

//Funzione principale che gestisce le azioni dell'utente
void game_action(int action, int client_id){
    switch (action)
    {
    case CREATE:
        create_game();
        break;
    case LIST:
        get_list_game();
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





