#include "game.h"
#include "game_vector.h"

game_vector_t game_vector;
int list_increment_game_id = 0;

// Funzione principale che gestisce le azioni dell'utente
void game_action(void *arg)
{
    int client_id = *(int *)arg;
    int sd = client_id; // In questo contesto, assumiamo che client_id sia anche il socket descriptor

    while (1)
    {
        int action = 0;
        recv(sd, &action, sizeof(int), 0);

        switch (action)
        {
        case CREATE:
            create_game(client_id);
            break;
        case LIST:
            get_list_game(sd);
            break;
        case JOIN:
        { 
            int game_id;
            if (recv(sd, &game_id, sizeof(int), 0) <= 0) //leggo l'id della partita
                break; 
            //join_game(client_id, game_id, sd);
            break;
        }
        case MOVE:
        int game_id;
            if (recv(sd, &game_id, sizeof(int), 0) <= 0) //leggo l'id della partita
                break; 
            //move(client_id, game_id, sd);
            break;
        case REMATCH:
            //rematch();
            break;
        case APPROVE:
        {
            int game_id, response;
            recv(sd, &game_id, sizeof(int), 0);
            recv(sd, &response, sizeof(int), 0);
            //approve_join_request(game_id, sd, response);
            break;
        }
        default:
            break;
        }
    }
}


void init_game_session(){

    init_game_vector(&game_vector);

}

void close_game_session(){

    destroy_game_vector(&game_vector);

}

void get_list_game(int sd)
{
    char buffer[4096] = "";

    for (int i = 0; i < game_vector.size; i++)
    {
        if (game_vector.vector[i] != NULL)
        {
            char game_info[256];
            sprintf(game_info, " %d; %d; %d;\n", game_vector.vector[i]->id, game_vector.vector[i]->id_player1, game_vector.vector[i]->state);
            strcat(buffer, game_info);
        }
    }

    send(sd, buffer, strlen(buffer), 0);
}

void create_game(int client_id)
{
    Game new_game = generate_game(client_id);

    insert_game_into_vector(&game_vector, &new_game);
}



void join_game(int client_id, int game_id, int sd)
{

    Game *selected_game = get_game_by_id(game_id);

    if(selected_game == NULL)
    {
        send(sd, "JOIN_ERR_NOT_FOUND", 18, 0);
        return;
    }

    pthread_mutex_lock(&selected_game->game_mutex);

    if (selected_game->state != ST_WAITING)
    {
        pthread_mutex_unlock(&selected_game->game_mutex);
        send(sd, "JOIN_ERR_FULL", 13, 0);
        return;
    }


    // ora la fase di approvazione della partita
    selected_game->id_player2 = client_id;
    selected_game->state = ST_APPROVE;

    send(selected_game->id_player1, "JOIN_REQUEST\n", 13, 0);

    // Questo while serve per attendere che il player 1 approvi o meno la richiesta di join
    while (selected_game->state == ST_APPROVE)
    {
        pthread_cond_wait(&selected_game->cond_approve, &selected_game->game_mutex);
    }

    // Una volta svegliato il thread controllo lo stato della partita
    if (selected_game->state == ST_PLAYING)
    {
        send(sd, "JOIN_OK", 7, 0);
    }
    else
    {
        send(sd, "JOIN_DENIED", 11, 0);
        selected_game->id_player2 = -1;
    }

    pthread_mutex_unlock(&selected_game->game_mutex);
}

void approve_join_request(int game_id, int sd, int response)
{

    Game *selected_game = get_game_by_id(game_id);

    if(selected_game == NULL)
    {
        send(sd, "APPROVE_ERR_NOT_FOUND", 21, 0);
        return;
    }

    pthread_mutex_lock(&selected_game->game_mutex);
    if (selected_game->state == ST_APPROVE)
    {

        if (response == 0)
        { // 0 approvato; 1 non approvato
            selected_game->state = ST_PLAYING;
        }
        else
        {
            selected_game->state = ST_WAITING;
        }

        //libera il thread del giocatore in attesa della risposta
        pthread_cond_signal(&selected_game->cond_approve);
    }

    pthread_mutex_unlock(&selected_game->game_mutex);
}

void move(int client_id, int game_id, int sd)
{
    int row, col;
    Game *selected_game = game_vector.vector[game_id];
    pthread_mutex_lock(selected_game->game_mutex);

    recv(sd, &row, sizeof(int), 0);
    recv(sd, &col, sizeof(int), 0);

    if(selected_game->table[row][col] != ' '){
        pthread_mutex_unlock(&selected_game->game_mutex);
        send(sd, "MOVE_INVALID", 12, 0);
        return;
    }

    if(selected_game->turn%2 == 0){ //pari player 1; dispari player 2
        if(selected_game->id_player1 == client_id){
            selected_game->table[row][col] = 'X';
            selected_game->turn++;
        }else{
             pthread_mutex_unlock(&selected_game->game_mutex);
            send(sd, "NOT_YOUR_TURN", 13, 0);
            return;
        }
    } else{
        if(selected_game->id_player2 == client_id){
            selected_game->table[row][col] = 'O';
            selected_game->turn++;
        }else{
             pthread_mutex_unlock(&selected_game->game_mutex);
            send(sd, "NOT_YOUR_TURN", 13, 0);
            return;
        }
    }

    pthread_mutex_unlock(selected_game->game_mutex);
    
}

int check_winner(Game* game)
{
    // Controlla righe e colonne
    for (int i = 0; i < 3; i++)
    {
        if (game->table[i][0] == game->table[i][1] && game->table[i][1] == game->table[i][2] && game->table[i][0] != ' ')
            return (game->table[i][0] == 'X') ? game->id_player1 : game->id_player2;
        if (game->table[0][i] == game->table[1][i] && game->table[1][i] == game->table[2][i] && game->table[0][i] != ' ')
            return (game->table[0][i] == 'X') ? game->id_player1 : game->id_player2;
    }

    // Controlla diagonali
    if (game->table[0][0] == game->table[1][1] && game->table[1][1] == game->table[2][2] && game->table[0][0] != ' ')
        return (game->table[0][0] == 'X') ? game->id_player1 : game->id_player2;
    if (game->table[0][2] == game->table[1][1] && game->table[1][1] == game->table[2][0] && game->table[0][2] != ' ')
        return (game->table[0][2] == 'X') ? game->id_player1 : game->id_player2;

    // Controlla pareggio
    int empty_found = 0;
    for (int r = 0; r < 3; r++)
    {
        for (int c = 0; c < 3; c++)
        {
            if (game->table[r][c] == ' ')
            {
                empty_found = 1;
                break;
            }
        }
        if (empty_found)
            break;
    }

    if (!empty_found)
        return 0; // Pareggio

    return -1; // Nessun vincitore ancora
}

Game generate_game(int client_id)
{
    // inizializzazione game
    Game new_game;
    new_game.id = ++list_increment_game_id;
    new_game.id_player1 = client_id;
    new_game.id_player2 = -1; // non esiste ancora
    new_game.turn = 0;
    new_game.state = ST_WAITING;

    // inizializzazione tabellone
    for (int r = 0; r < 3; r++)
    {
        for (int c = 0; c < 3; c++)
        {
            new_game.table[r][c] = ' ';
        }
    }

    // inizializzazione del mutex
    pthread_mutex_init(&new_game.game_mutex, NULL);
    pthread_cond_init(&new_game.cond_approve, NULL);
    return new_game;
}
