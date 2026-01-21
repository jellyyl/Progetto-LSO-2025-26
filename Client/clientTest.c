#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

// Operazioni dei giocatori
#define CMD_PLAY 1
#define CMD_WAIT 2
#define CMD_OVER 3 //fine partita
#define CMD_INVALID 4 //mossa non valida

// Definizioni coerenti con Actions in game.h
#define CREATE 1
#define LIST 2
#define JOIN 3
#define APPROVE 6
#define EXIT 0

#define SERVER_IP "127.0.0.1"
#define PORT 5200

void send_move(int sd, int game_id, int row, int col) {
    int action = 4; // MOVE
    send(sd, &action, sizeof(int), 0);
    send(sd, &game_id, sizeof(int), 0);
    send(sd, &row, sizeof(int), 0);
    send(sd, &col, sizeof(int), 0);
}

void game(int sd, int game_id) {
    int row, col;
    char board_buffer[1024];
    int command; 

    printf("\n--- GIOCO INIZIATO ---\n");

    while(1) {
       
        //lettura comando dal server
        if (recv(sd, &command, sizeof(int), 0) <= 0) break;

        memset(board_buffer, 0, sizeof(board_buffer));
        if (recv(sd, board_buffer, sizeof(board_buffer)-1, 0) <= 0) break;

        printf("%s\n", board_buffer);

        if (command == CMD_OVER) {
            printf("La partita è terminata. Torno al menu.\n");
            break; 
        }
        else if (command == CMD_WAIT) {
            printf("In attesa della mossa dell'avversario...\n");
            continue; 
        }
        else if (command == CMD_PLAY) {

            printf("È il tuo turno! Inserisci riga e colonna (es: 0 1): ");
            scanf("%d %d", &row, &col);
            send_move(sd, game_id, row, col);
        }
        else if (command == CMD_INVALID) {
            printf("\n!!! ATTENZIONE: Mossa non valida o non è il tuo turno !!!\n");
            printf("Riprova. Inserisci riga e colonna (es: 0 1): ");
            scanf("%d %d", &row, &col);
            send_move(sd, game_id, row, col);
        }
    }
}



void print_menu() {
    printf("\n--- MENU TRIS INTERATTIVO ---\n");
    printf("1. Crea una nuova partita (CREATE)\n");
    printf("2. Visualizza lista partite (LIST)\n");
    printf("3. Unisciti a una partita (JOIN)\n");
    printf("0. Esci\n");
    printf("Scelta: ");
}

int main() {
    int sd;
    struct sockaddr_in server_addr;
    int scelta;
    char buffer[4096];

    // Creazione del socket
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0) {
        perror("Errore socket");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT); // Porta 5200
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // Connessione al server
    if (connect(sd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connessione fallita");
        close(sd);
        return 1;
    }

    printf("Connesso al server.\n");

    while (1) {
        print_menu();
        if (scanf("%d", &scelta) != 1) break;

        if (scelta == EXIT) break;

        // Invio dell'azione come intero
        send(sd, &scelta, sizeof(int), 0);

        if (scelta == CREATE) {
            // Il server esegue create_game() ma non invia conferme
            printf("\n------------------------------------------------\n");
            printf(" Partita creata! Sei in 'Sala d'Attesa'.\n");
            printf(" Non toccare nulla finché non arriva uno sfidante.\n");
            printf("------------------------------------------------\n");

            memset(buffer, 0, sizeof(buffer)); //pulisco il buffer
            int n = recv(sd, buffer, sizeof(buffer) - 1, 0); 
            printf("Ricevuto dal server: %d\n", n);
            printf("Ricevuto dal server (buffer): %s\n", buffer);

            //risveglio
            if (n > 0 && strstr(buffer, "JOIN_REQUEST")) {
                printf("\n>>> NOTIFICA: %s\n", buffer);
                
                int game_id, resp, game_action;
                
                memcpy(&game_id, buffer + 13, sizeof(int)); //estraggo game_id
                
                printf("Un giocatore vuole unirsi alla tua partita (ID: %d).\n", game_id);
                printf("Accetti la richiesta? (0 = Sì, 1 = No): ");

                scanf("%d", &resp);
                game_action = APPROVE;
                send(sd, &game_action, sizeof(int), 0); 
                send(sd, &game_id, sizeof(int), 0); 
                send(sd, &resp, sizeof(int), 0); 

                memset(buffer, 0, sizeof(buffer));
                recv(sd, buffer, sizeof(buffer) - 1, 0); //ricevo esito (START_PLAYER1 o CANCELLED)
                printf("Stato partita: %s\n", buffer);
                if (strstr(buffer, "START_PLAYER1")) {
                    printf("La partita sta per iniziare!\n");
                    game(sd, game_id);
                }
                
            }
            else {
                printf("Errore o connessione chiusa dal server.\n");
            }

        } 
        else if (scelta == LIST) {
            printf("Richiedo la lista...\n");
            memset(buffer, 0, sizeof(buffer));
            
            // Il server invia una stringa formattata con i dati del vettore
            int n = recv(sd, buffer, sizeof(buffer) - 1, 0);
            if (n > 0) {
                printf("\n--- PARTITE DISPONIBILI ---\n");
                printf("ID; ID_P1; STATO;\n");
                printf("%s", buffer);
            } else {
                printf("Nessuna partita attiva o errore di ricezione.\n");
            }
        } else if (scelta == JOIN) {
            int game_id;
            printf("Inserisci l'ID della partita a cui vuoi unirti: ");
            scanf("%d", &game_id);

            //Invia l'ID della partita
            send(sd, &game_id, sizeof(int), 0);

            printf("Richiesta inviata. In attesa di approvazione dal Player 1...\n");

          
            
            memset(buffer, 0, sizeof(buffer));
            int n = recv(sd, buffer, sizeof(buffer) - 1, 0); 
            if (n > 0) {
                printf("Risposta server: %s\n", buffer);
                if (strstr(buffer, "JOIN_OK")) {
                    printf("La partita sta per iniziare!\n");
                    game(sd, game_id);
                } else {
                    printf("Impossibile unirsi alla partita. Potrebbe essere stata cancellata o non trovata.\n");
                }
            }
            
        }
        else {
            printf("Azione non valida.\n");
        }
    }

    close(sd);
    printf("Client chiuso.\n");
    return 0;
}

