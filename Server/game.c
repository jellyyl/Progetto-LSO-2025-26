#include "game.h"
#include "game_vector.h"
#include <stdio.h>      
#include <stdlib.h>     
#include <string.h>    
#include <sys/socket.h> 
#include <unistd.h>     
#include <arpa/inet.h> 

game_vector_t game_vector;
int list_increment_game_id = 0;

void move(int game_id, int sd);
void send_board_to_socket(int sd, Game* game, GameCommand cmd);

int check_winner(Game* game);
int create_game(int client_id);
Game generate_game(int client_id);
void get_list_game(int sd);
void join_game(int client_id, int game_id, int sd);
void approve_join_request(int game_id, int sd, int response);
void broadcast_game_state(Game *game, int check_error);
void quit_game(int disconnected_player, int game_id);

// Funzione principale che gestisce le azioni dell'utente
void game_action(void *arg)
{
    int client_id = *(int *)arg;
    int sd = client_id; 
    int current_game_id = -1;
    while (1)
    {
        int action = 0;
        //ogni volta controllo la disconnessione 
        if (recv(sd, &action, sizeof(int), 0) <= 0) {
            quit_game(sd, current_game_id); 
            break; 
        }
        
        switch (action)
        {
        case CREATE:
            current_game_id = create_game(client_id);
            break;
        case LIST:
            get_list_game(sd);
            break;
        case JOIN:
        {          
            int game_id;
            if (recv(sd, &game_id, sizeof(int), 0) <= 0) 
                break; 
            current_game_id = game_id;
            join_game(client_id, game_id, sd);
            break;
        }
        case MOVE:
        {
            int game_id;
            if (recv(sd, &game_id, sizeof(int), 0) <= 0) 
                break; 
            
            current_game_id = game_id;
            move(game_id, sd);
            break;
        }
        case REMATCH: 
        {
            int game_id;
            if (recv(sd, &game_id, sizeof(int), 0) <= 0) 
                break; 
            do_rematch(game_id, sd);
            break;
        }
        case APPROVE:
        {
            int game_id, response;
            recv(sd, &game_id, sizeof(int), 0);
            recv(sd, &response, sizeof(int), 0);
            if(response == 0){current_game_id = game_id;}
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

void quit_game(int disconnected_player, int game_id){

    Game *game = get_game_by_id(&game_vector, game_id);
    if (game == NULL){
        printf("Errore nella ricerca della partita (ID %d).\n", game_id);
        close(disconnected_player);
        return;
    }

    pthread_mutex_lock(&game->game_mutex);

    if(game->state == ST_WAITING){
        game->state = ST_FINISHED;
        printf("P1 si è disconnesso dalla lobby %d. Partita chiusa.\n", game_id);
        pthread_cond_broadcast(&game->cond_wait_P1);
    }
    else if(game->state == ST_APPROVE || game->state == ST_PLAYING){
        
        int player_in_game = -1;
        if(disconnected_player == game->id_player1) player_in_game = game->id_player2;
        else if(disconnected_player == game->id_player2) player_in_game = game->id_player1;

        if (player_in_game != -1) { 
            if (game->state == ST_PLAYING) {
                int cmd = CMD_QUIT;
                send(player_in_game, &cmd, sizeof(int), 0);
            }
        }
        
        game->state = ST_FINISHED; 
        printf("Socket %d disconnesso. Partita %d terminata forzatamente.\n", disconnected_player, game->id);
        
       
        pthread_cond_broadcast(&game->cond_approve);
    }    
    pthread_mutex_unlock(&game->game_mutex);
    
    remove_game_by_id(&game_vector, game_id);
    close(disconnected_player);
}

int create_game(int client_id)
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
        return new_game.id;
    } else{
        printf("Errore nell'inserimento della nuova partita.\n");
        return -1;
    }
}

void join_game(int client_id, int game_id, int sd)
{
    Game *selected_game = get_game_by_id(&game_vector, game_id);
    printf("Player %d tenta di unirsi alla partita %d...\n", client_id, game_id);

    if(selected_game == NULL) {
        int err = ERR_JOIN_NOT_FOUND;
        send(sd, &err, sizeof(int), 0);
        return;
    }

    pthread_mutex_lock(&selected_game->game_mutex);

    if (selected_game->state != ST_WAITING) {
        pthread_mutex_unlock(&selected_game->game_mutex);
        int err = MSG_JOIN_DENIED;
        send(sd, &err, sizeof(int), 0); 
        return;
    }

    selected_game->id_player2 = client_id;
    selected_game->state = ST_APPROVE;
    
    // Notifica P1
    int p1_sd = selected_game->id_player1;
    int join_packet[2];
    join_packet[0] = MSG_JOIN_REQUEST;
    join_packet[1] = game_id;
    
    if ((send(p1_sd, join_packet, sizeof(join_packet), MSG_NOSIGNAL) < 0)) { // se la send a p1 non va buon fine, P1 si è disconesso
        
        printf("Rilevato P1 disconnesso...\n");
    
        selected_game->state = ST_FINISHED;
        
        int msg_error = ERR_JOIN_OWNER_LEFT;
        send(sd, &msg_error, sizeof(int), 0);
        
        selected_game->id_player2 = -1;

        pthread_mutex_unlock(&selected_game->game_mutex);
        return;
    }
    
    pthread_cond_signal(&selected_game->cond_wait_P1);

    while (selected_game->state == ST_APPROVE) {
        pthread_cond_wait(&selected_game->cond_approve, &selected_game->game_mutex);
    }

    if (selected_game->state == ST_PLAYING) {
        int msg = MSG_JOIN_OK;
        send(sd, &msg, sizeof(int), 0);
        usleep(100000);
        int cmd = CMD_WAIT; 
        send_board_to_socket(sd, selected_game, cmd);
    } 
    else if (selected_game->state == ST_FINISHED) {
        
        int err = MSG_JOIN_DENIED;
        send(sd, &err, sizeof(int), 0);
        selected_game->id_player2 = -1;
    }
    else {
        int err = MSG_JOIN_DENIED;
        send(sd, &err, sizeof(int), 0);
        selected_game->id_player2 = -1;
    }

    pthread_mutex_unlock(&selected_game->game_mutex);
}

void approve_join_request(int game_id, int sd, int response)
{
    Game *selected_game = get_game_by_id(&game_vector, game_id);

    if(selected_game == NULL) {
        int err = ERR_APPROVE_NOT_FOUND;
        send(sd, &err, sizeof(int), 0);
        return;
    }

    pthread_mutex_lock(&selected_game->game_mutex);
    if (selected_game->state == ST_APPROVE)
    {
        if (response == 0)
        { 
            selected_game->state = ST_PLAYING;
            int msg = MSG_START_PLAYER1;
            send(sd, &msg, sizeof(int), 0);
            usleep(100000); 
            // Inizializzazione Manuale P1 
            int cmd = CMD_PLAY; 
            //send(sd, &cmd, sizeof(int), 0);
            send_board_to_socket(sd, selected_game, cmd); 
        }
        else
        {
            selected_game->state = ST_WAITING;
            int msg = MSG_CANCELLED;
            send(sd, &msg, sizeof(int), 0);
        }
        pthread_cond_signal(&selected_game->cond_approve);
    }
    pthread_mutex_unlock(&selected_game->game_mutex);
}



void send_board_to_socket(int sd, Game* game, GameCommand cmd) {
    char board_update[1024];
    
    memcpy(board_update, &cmd, sizeof(int));
    sprintf(board_update + sizeof(int),
           "%c%c%c%c%c%c%c%c%c",         
            game->table[0][0], game->table[0][1], game->table[0][2],
            game->table[1][0], game->table[1][1], game->table[1][2],
            game->table[2][0], game->table[2][1], game->table[2][2]);

    send(sd, board_update, sizeof(int) + strlen(board_update + sizeof(int)), 0);
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

        int winner = check_winner(game);
        char msg_p1[100];
        char msg_p2[100];

        if (winner == 0) {
            cmd_p1 = CMD_DRAW;
            cmd_p2 = CMD_DRAW;
        } else if (winner == game->id_player1) {
            cmd_p1 = CMD_WIN;
            cmd_p2 = CMD_LOSE;
        } else {
            cmd_p1 = CMD_LOSE;
            cmd_p2 = CMD_WIN;
        }

        send_board_to_socket(game->id_player1, game, cmd_p1);
    
        if (game->id_player2 != -1) {
            send_board_to_socket(game->id_player2, game, cmd_p2);
        }

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
    send_board_to_socket(game->id_player1, game, cmd_p1);

    if (game->id_player2 != -1) {
        send_board_to_socket(game->id_player2, game, cmd_p2);
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

    if(status != -1) {
        game_over(selected_game, status);
    }

    pthread_mutex_unlock(&selected_game->game_mutex);
}


int do_rematch(int game_id, int sd){

    Game* selected_game = get_game_by_id(&game_vector, game_id);
    int response;
    

    if(!selected_game){
        int err = ERR_REMATCH_NOT_FOUND;
        send(sd, &err, sizeof(int), 0);
        printf("Rematch: partita non trovata (ID %d)\n", game_id);
        return -1;
    }

    if(selected_game->id_player1 != sd && selected_game->id_player2 != sd){
        int err = ERR_REMATCH_NOT_PLAYER;
        send(sd, &err, sizeof(int), 0);
        printf("Rematch: giocatore non nella partita (ID %d)\n", game_id);
        return -1;
    }

    if(selected_game->state != ST_FINISHED){
        int err = ERR_REMATCH_NOT_FINISHED;
        send(sd, &err, sizeof(int), 0);
        printf("Rematch: partita non finita (ID %d)\n", game_id);
        return -1;
    }

    recv(sd, &response, sizeof(int), 0);


    //significa che c'è stato un vincitore
    if(selected_game->id_player2 == -1){
        rematch_by_winner(selected_game, sd, response);
        return 0;
    } else { // pareggio
        rematch_from_both(selected_game, sd, response);
        return 0;
    }
}

int rematch_by_winner(Game* game, int sd, int response){

    int cmd_message;

    if(game->id_player1 != sd){
        int err = ERR_REMATCH_NOT_OWNER;
        send(sd, &err, sizeof(int), 0);
        return -1;
    }

    //bisogna cambiare e gestire la fine della partita
    if(response != 1) {
        cmd_message = CMD_OVER;
        remove_game_by_id(&game_vector, game->id);
        send(sd, &cmd_message, sizeof(int), 0);
        return 0;
    }

    pthread_mutex_lock(&game->game_mutex);

    clear_game(game);

    printf("Player 1 (ID %d) bloccato in attesa di sfidanti...\n", sd);
    while (game->state == ST_WAITING) {
        pthread_cond_wait(&game->cond_wait_P1, &game->game_mutex);
    }
    pthread_mutex_unlock(&game->game_mutex);

    return 0;

}

int rematch_from_both( Game* game, int sd, int response){
    
    pthread_mutex_lock(&game->game_mutex);



    if(sd == game->id_player1){
        game->rematch_status_player1 = response;
        if(response == 0) {
            int msg = MSG_REMATCH_DECLINED;
            send(game->id_player2, &msg, sizeof(int), 0);
        }

    } else if (sd == game->id_player2){
        game->rematch_status_player2 = response;
        if(response == 0) {
            int msg = MSG_REMATCH_DECLINED;
            send(game->id_player1, &msg, sizeof(int), 0);
        }
    }

    if(response == 0){

        pthread_mutex_unlock(&game->game_mutex);
        pthread_cond_broadcast(&game->cond_approve);

        if(game->rematch_status_player1 == 0 && game->rematch_status_player2 == 0){
            remove_game_by_id(&game_vector, game->id);
        }

        return 0;
    }

    pthread_cond_broadcast(&game->cond_approve);

    // il primo che si libera, pulisce il game e i status tornano a -1, è necessario quindi mettere la terza condizione nel while
    while ((game->rematch_status_player1 == -1 || game->rematch_status_player2 == -1) && game->state != ST_PLAYING) {
        pthread_cond_wait(&game->cond_approve, &game->game_mutex); //aspetta e sblocca il mutex "mentre dorme"

        //se uno dei due ha rifiutato
        if(game->rematch_status_player1 == 0 || game->rematch_status_player2 == 0){

            pthread_mutex_unlock(&game->game_mutex);
            remove_game_by_id(&game_vector, game->id);
            return -1;
        }
  
    }
    printf("Entrambi i giocatori hanno accettato la rivincita per la partita %d!\n", game->id);

    if (game->state != ST_PLAYING) {
        printf("Entrambi i giocatori hanno accettato la rivincita per la partita %d!\n", game->id);
        clear_game(game);          // Attenzione: questo resetta i flag a -1
        game->state = ST_PLAYING;  
    }
    
    if(game->id_player1 == sd){
        int msg = MSG_START_PLAYER1;
        send(sd, &msg, sizeof(int), 0);
        usleep(100000); 
        // Inizializzazione Manuale P1 
        int cmd_p1 = CMD_PLAY; 

        send_board_to_socket(sd, game, cmd_p1);
    } else if (game->id_player2 == sd){
        int msg = MSG_START_PLAYER2;
        send(sd, &msg, sizeof(int), 0);
        usleep(100000); 
        // Inizializzazione Manuale P2 
        int cmd_p2 = CMD_WAIT; 

        send_board_to_socket(sd, game, cmd_p2);
    }

    pthread_mutex_unlock(&game->game_mutex);

    return 0;

}


Game generate_game(int client_id)
{
    Game new_game;
    //gestire il mutex per il contatore globale per gli id
    new_game.id = ++list_increment_game_id;
    new_game.id_player1 = client_id;
    new_game.id_player2 = -1; 
    new_game.turn = 0;
    new_game.state = ST_WAITING;
    new_game.rematch_status_player1 = -1;
    new_game.rematch_status_player2 = -1;   

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


int game_over(Game* game, int winner_sd){

    if(!game){
        return -1;
    }

    //se il vincitore non è il giocatore 1, cambia il proprietario
    if(winner_sd == game->id_player2){
        change_owner_game(game);
        game->id_player2 = -1;
        int msg = MSG_CHANGE_OWNER;
        send(game->id_player1, &msg, sizeof(int), 0);
    } else if(winner_sd == game->id_player1){
        game->id_player2 = -1; 
    }
    

    usleep(50000);

    return ask_rematch(game, winner_sd);
}

int change_owner_game(Game* game){

    int tempSd = game->id_player1;
    game->id_player1 = game->id_player2;
    game->id_player2 = tempSd;
    return 0;
}

int ask_rematch(Game* game, int winner_sd){

    if(!game){
        return -1;
    }

    int msg = MSG_REMATCH_REQUEST;

    if(winner_sd == 0){
        send(game->id_player1, &msg, sizeof(int), 0);
        send(game->id_player2, &msg, sizeof(int), 0);
    } else { //lo mando solo al proprietario (ossia il vincitore)
        send(game->id_player1, &msg, sizeof(int), 0);
    } 

    return 0;
}

int clear_game(Game *game){

    if(!game){
        return -1;
    }

    game->state = ST_WAITING;
    game->turn = 0;
    game->rematch_status_player1 = -1;
    game->rematch_status_player2 = -1;

    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            game->table[r][c] = ' ';
        }
    }

    return 0;
}