#include "game.h"
#include "game_vector.h"
#include <stdio.h>      
#include <stdlib.h>     
#include <string.h>    
#include <sys/socket.h> 
#include <unistd.h>     
#include <arpa/inet.h> 

// Codici di comando dal server
#define CMD_PLAY 1
#define CMD_WAIT 2
#define CMD_OVER 3
#define CMD_INVALID 4 // mossa non valida

game_vector_t game_vector;
int list_increment_game_id = 0;

void move(int game_id, int sd);
void send_board_to_socket(int sd, Game* game);
void send_board_with_message(int sd, Game* game, char* msg); 
int check_winner(Game* game);
void create_game(int client_id);
Game generate_game(int client_id);
void get_list_game(int sd);
void join_game(int client_id, int game_id, int sd);
void approve_join_request(int game_id, int sd, int response);
void broadcast_game_state(Game *game, int check_error);

// Funzione principale che gestisce le azioni dell'utente
void game_action(void *arg)
{
    int client_id = *(int *)arg;
    int sd = client_id; 

    while (1)
    {
        int action = 0;
        if (recv(sd, &action, sizeof(int), 0) <= 0) break;

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
            if (recv(sd, &game_id, sizeof(int), 0) <= 0) 
                break; 
            join_game(client_id, game_id, sd);
            break;
        }
        case MOVE:
        {
            int game_id;
            if (recv(sd, &game_id, sizeof(int), 0) <= 0) 
                break; 
            move(game_id, sd);
            break;
        }
        case REMATCH:
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
    Game *inserted_game = get_game_by_id(&game_vector, new_game.id);

    if(inserted_game != NULL){
        pthread_mutex_lock(&inserted_game->game_mutex);
        
        printf("Player 1 (ID %d) bloccato in attesa di sfidanti...\n", client_id);
        while (inserted_game->state == ST_WAITING) {
            pthread_cond_wait(&inserted_game->cond_wait_P1, &inserted_game->game_mutex);
        }
        pthread_mutex_unlock(&inserted_game->game_mutex);
    } else{
        printf("Errore nell'inserimento della nuova partita.\n");
    }
}

void join_game(int client_id, int game_id, int sd)
{
    Game *selected_game = get_game_by_id(&game_vector, game_id);
    printf("Player %d tenta di unirsi alla partita %d...\n", client_id, game_id);

    if(selected_game == NULL) {
        send(sd, "JOIN_ERR_NOT_FOUND", 18, 0);
        return;
    }

    pthread_mutex_lock(&selected_game->game_mutex);

    if (selected_game->state != ST_WAITING) {
        pthread_mutex_unlock(&selected_game->game_mutex);
        return;
    }

    selected_game->id_player2 = client_id;
    selected_game->state = ST_APPROVE;
    
    // Notifica P1
    int p1_sd = selected_game->id_player1;
    char join_packet[17];
    memset(join_packet, 0, 17);
    strcpy(join_packet, "JOIN_REQUEST");
    memcpy(join_packet + 13, &game_id, sizeof(int));
    send(p1_sd, join_packet, 17, 0);
    
    pthread_cond_signal(&selected_game->cond_wait_P1);

    while (selected_game->state == ST_APPROVE) {
        pthread_cond_wait(&selected_game->cond_approve, &selected_game->game_mutex);
    }

    if (selected_game->state == ST_PLAYING) {
        send(sd, "JOIN_OK", 7, 0);
        usleep(100000);
        // Inizializzazione Manuale P2 
        int cmd = CMD_WAIT; 
        send(sd, &cmd, sizeof(int), 0);
        send_board_to_socket(sd, selected_game);
        
    } else {
        send(sd, "JOIN_DENIED", 11, 0);
        selected_game->id_player2 = -1;
    }

    pthread_mutex_unlock(&selected_game->game_mutex);
}

void approve_join_request(int game_id, int sd, int response)
{
    Game *selected_game = get_game_by_id(&game_vector, game_id);

    if(selected_game == NULL) {
        send(sd, "APPROVE_ERR_NOT_FOUND", 21, 0);
        return;
    }

    pthread_mutex_lock(&selected_game->game_mutex);
    if (selected_game->state == ST_APPROVE)
    {
        if (response == 0)
        { 
            selected_game->state = ST_PLAYING;
            send(sd, "START_PLAYER1", 14, 0);
            usleep(100000);
            // Inizializzazione Manuale P1 
            int cmd = CMD_PLAY; 
            send(sd, &cmd, sizeof(int), 0);
            send_board_to_socket(sd, selected_game); 
        }
        else
        {
            selected_game->state = ST_WAITING;
            send(sd, "CANCELLED", 10, 0);
        }
        pthread_cond_signal(&selected_game->cond_approve);
    }
    pthread_mutex_unlock(&selected_game->game_mutex);
}

// invia SOLO il tabellone (usata durante la partita)
void send_board_to_socket(int sd, Game* game) {
    char board_update[1024];
    sprintf(board_update, 
            "\n"
            "   0   1   2\n"              
            "0  %c | %c | %c \n"          
            "  ---|---|---\n"             
            "1  %c | %c | %c \n"          
            "  ---|---|---\n"             
            "2  %c | %c | %c \n",         
            game->table[0][0], game->table[0][1], game->table[0][2],
            game->table[1][0], game->table[1][1], game->table[1][2],
            game->table[2][0], game->table[2][1], game->table[2][2]);

    send(sd, board_update, strlen(board_update), 0);
}

// invia Tabellone + MESSAGGIO (usata a fine partita)
void send_board_with_message(int sd, Game* game, char* msg) {
    char full_message[2048]; 
    sprintf(full_message, 
            "\n"
            "   0   1   2\n"              
            "0  %c | %c | %c \n"          
            "  ---|---|---\n"             
            "1  %c | %c | %c \n"          
            "  ---|---|---\n"             
            "2  %c | %c | %c \n"
            "\n%s\n", // Messaggio attaccato in fondo
            game->table[0][0], game->table[0][1], game->table[0][2],
            game->table[1][0], game->table[1][1], game->table[1][2],
            game->table[2][0], game->table[2][1], game->table[2][2],
            msg);

    send(sd, full_message, strlen(full_message), 0);
}

int check_winner(Game* game)
{
    // Controlla righe
    for (int i = 0; i < 3; i++) {
        if (game->table[i][0] == game->table[i][1] && game->table[i][1] == game->table[i][2] && game->table[i][0] != ' ')
            return (game->table[i][0] == 'X') ? game->id_player1 : game->id_player2;
    }
    // Controlla colonne
    for (int i = 0; i < 3; i++) {
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
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            if (game->table[r][c] == ' ') {
                empty_found = 1;
                break;
            }
        }
    }

    if (!empty_found) return 0; 
    return -1; 
}

void broadcast_game_state(Game *game, int check_error) {
    int cmd_p1, cmd_p2;

    //partita finita
    if (game->state == ST_FINISHED) {
        cmd_p1 = CMD_OVER;
        cmd_p2 = CMD_OVER;

        int winner = check_winner(game);
        char msg_p1[100];
        char msg_p2[100];

        if (winner == 0) {
            sprintf(msg_p1, "--- NON CI CREDO FUNZIONA ANCHE IL PAREGGIO ---");
            sprintf(msg_p2, "--- NON CI CREDO FUNZIONA ANCHE IL PAREGGIO ---");
        } else if (winner == game->id_player1) {
            sprintf(msg_p1, "🏆 Complimenti gennaro de luca ha finito il game del tris 🏆");
            sprintf(msg_p2, "💀 PAR O' CAZZ  💀");
        } else {
            sprintf(msg_p1, "💀 PAR O' CAZZ 💀");
            sprintf(msg_p2, "🏆 Complimenti gennaro de luca ha finito il game del tris 🏆");
        }

        // Usa send_board_with_message per inviare tabellone + testo
        send(game->id_player1, &cmd_p1, sizeof(int), 0);
        send_board_with_message(game->id_player1, game, msg_p1);

        if (game->id_player2 != -1) {
            send(game->id_player2, &cmd_p2, sizeof(int), 0);
            send_board_with_message(game->id_player2, game, msg_p2);
        }

        game_over(game->id);

        return; 
    }

    // mossa non valida
    if (check_error == 1) { 
        int current_turn_player = (game->turn % 2 == 0) ? game->id_player1 : game->id_player2;
        if (current_turn_player == game->id_player1) {
            cmd_p1 = CMD_INVALID; cmd_p2 = CMD_WAIT;
        } else {
            cmd_p1 = CMD_WAIT; cmd_p2 = CMD_INVALID;
        }
    } 
    // mossa valida
    else {
        int current_turn_player = (game->turn % 2 == 0) ? game->id_player1 : game->id_player2;
        if (current_turn_player == game->id_player1) {
            cmd_p1 = CMD_PLAY; cmd_p2 = CMD_WAIT;
        } else {
            cmd_p1 = CMD_WAIT; cmd_p2 = CMD_PLAY;
        }
    }

    // Invio Standard
    send(game->id_player1, &cmd_p1, sizeof(int), 0);
    send_board_to_socket(game->id_player1, game);

    if (game->id_player2 != -1) {
        send(game->id_player2, &cmd_p2, sizeof(int), 0);
        send_board_to_socket(game->id_player2, game);
    }
}

void move(int game_id, int sd)
{
    int row, col;
    int check_error = 0;
    
    if (recv(sd, &row, sizeof(int), 0) <= 0 || recv(sd, &col, sizeof(int), 0) <= 0) return;

    Game *selected_game = get_game_by_id(&game_vector, game_id);
    if (selected_game == NULL) return;

    pthread_mutex_lock(&selected_game->game_mutex);

    // Controllo Limiti
    if (row < 0 || row > 2 || col < 0 || col > 2) {
        check_error = 1; 
        broadcast_game_state(selected_game, check_error); 
        pthread_mutex_unlock(&selected_game->game_mutex);
        return;
    }

    // Controllo Validità Mossa
    int current_player_sd = (selected_game->turn % 2 == 0) ? selected_game->id_player1 : selected_game->id_player2;
    if (selected_game->table[row][col] != ' ' || sd != current_player_sd) {
        check_error = 1; 
        broadcast_game_state(selected_game, check_error); 
        pthread_mutex_unlock(&selected_game->game_mutex);
        return;
    }

    selected_game->table[row][col] = (sd == selected_game->id_player1) ? 'X' : 'O';
    selected_game->turn++; 

    int status = check_winner(selected_game);
    if (status != -1) { 
        selected_game->state = ST_FINISHED;        
    }

    //invia aggiornamento stato a entrambi i giocatori
    broadcast_game_state(selected_game, 0);

    pthread_mutex_unlock(&selected_game->game_mutex);
}

Game generate_game(int client_id)
{
    Game new_game;
    new_game.id = ++list_increment_game_id;
    new_game.id_player1 = client_id;
    new_game.id_player2 = -1; 
    new_game.turn = 0;
    new_game.state = ST_WAITING;

    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            new_game.table[r][c] = ' ';
        }
    }

    pthread_mutex_init(&new_game.game_mutex, NULL);
    pthread_cond_init(&new_game.cond_approve, NULL);
    pthread_cond_init(&new_game.cond_wait_P1, NULL);
    return new_game;
}


int game_over(int game_id){

    return remove_game_by_id(&game_vector, game_id);

}