#include "game.h"

// siccome usiamo extern dobbiamo inizializzare le variabili qui
int list_increment_game_id = 0;
pthread_mutex_t mutex_lista = PTHREAD_MUTEX_INITIALIZER;
Game *list_game[LIST_INIT_SIZE] = {NULL};

// utilizzare un arrays di gamesì globale per la lista delle partire

// Funzione principale che gestisce le azioni dell'utente
void game_action(int client_id, int sd)
{

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
            join_game(client_id, game_id, sd);
            break;
        }
        case MOVE:
            move_character();
            break;
        case REMATCH:
            rematch();
            break;
        case APPROVE:
        {
            int game_id, response;
            recv(sd, &game_id, sizeof(int), 0);
            recv(sd, &response, sizeof(int), 0);
            approve_join_request(game_id, sd, response);
            break;
        }
        default:
            break;
        }
    }
}

void get_list_game(int sd)
{
    char buffer[2048] = "";

    for (int i = 0; i < LIST_INIT_SIZE; i++)
    {
        if (list_game[i] != NULL)
        {
            char game_info[256];
            sprintf(game_info, " %d; %d; %d;\n", list_game[i]->id, list_game[i]->id_player1, list_game[i]->state);
            strcat(buffer, game_info);
        }
    }

    send(sd, buffer, strlen(buffer), 0);
}

// con allocazione dinamica dell'array sarebbe complessa perchè dovrei bloccare tutti i thread e quindi bloccare anche le partite
void create_game(int client_id)
{
    pthread_mutex_lock(&mutex_lista);

    int found_index = -1;
    for (int i = 0; i < LIST_INIT_SIZE; i++)
    {
        if (list_game[i] == NULL)
        { // trovo il primo spazio libero
            found_index = i;
            break;
        }
    }

    if (found_index == -1)
    { // se ho la lista piena mando un errore
        pthread_mutex_unlock(&mutex_lista);
        return;
    }

    create_game_into_list(client_id, found_index);

    pthread_mutex_unlock(&mutex_lista);
}

void join_game(int client_id, int game_id, int sd)
{

    Game *selected_game = find_game_by_id(game_id);

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

    Game *selected_game = find_game_by_id(game_id);

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

        pthread_cond_signal(&selected_game->cond_approve);
    }

    pthread_mutex_unlock(&selected_game->game_mutex);
}

Game *find_game_by_id(int game_id)
{
    Game *selected_game = NULL;

    pthread_mutex_lock(&mutex_lista);

    for (int i = 0; i < LIST_INIT_SIZE; i++)
    {
        if (list_game[i] != NULL && list_game[i]->id == game_id)
        {
            selected_game = list_game[i];
            break;
        }
    }

    pthread_mutex_unlock(&mutex_lista);
    return selected_game;
}

void create_game_into_list(int client_id, int found_index)
{
    // inizializzazione game
    list_game[found_index] = malloc(sizeof(Game));
    list_game[found_index]->id = ++list_increment_game_id;
    list_game[found_index]->id_player1 = client_id;
    list_game[found_index]->id_player2 = -1; // non esiste ancora
    list_game[found_index]->turn = 0;
    list_game[found_index]->state = ST_WAITING;

    // inizializzazione tabellone
    for (int r = 0; r < 3; r++)
    {
        for (int c = 0; c < 3; c++)
        {
            list_game[found_index]->table[r][c] = ' ';
        }
    }

    // inizializzazione del mutex
    pthread_mutex_init(&list_game[found_index]->game_mutex, NULL);
}
