#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define CMD_PLAY 1
#define CMD_WAIT 2
#define CMD_OVER 3 
#define CMD_INVALID 4 

#define CREATE 1
#define LIST 2
#define JOIN 3
#define MOVE 4
#define REMATCH 5   
#define APPROVE 6
#define EXIT 0

#define SERVER_IP "127.0.0.1"
#define PORT 5200

void game_loop(int sd, int game_id);
int wait_for_challenger(int sd);
void clear_input();

void send_move(int sd, int game_id, int row, int col) {
    int action = MOVE; 
    send(sd, &action, sizeof(int), 0);
    send(sd, &game_id, sizeof(int), 0);
    send(sd, &row, sizeof(int), 0);
    send(sd, &col, sizeof(int), 0);
}

void clear_input() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Funzione per attendere uno sfidante (usata in CREATE e REMATCH VINCITORE)
int wait_for_challenger(int sd) {
    char buffer[4096];
    printf("\nIn attesa di sfidanti...\n");
    
    memset(buffer, 0, sizeof(buffer));
    int n = recv(sd, buffer, sizeof(buffer) - 1, 0); 
    
    if (n > 0 && strstr(buffer, "JOIN_REQUEST")) {
        int game_id, resp, action = APPROVE;
        memcpy(&game_id, buffer + 13, sizeof(int));
        
        printf("\n>>> SFIDA RICEVUTA (Partita %d). Accetti? (0=Si, 1=No): ", game_id);
        if(scanf("%d", &resp) != 1) resp = 1;
        clear_input();
        
        send(sd, &action, sizeof(int), 0); 
        send(sd, &game_id, sizeof(int), 0); 
        send(sd, &resp, sizeof(int), 0); 

        if (resp != 0) {
            printf("Sfida rifiutata.\n");
            return -1;
        }

        memset(buffer, 0, sizeof(buffer));
        recv(sd, buffer, sizeof(buffer) - 1, 0); 
        
        if (strstr(buffer, "START_PLAYER1")) {
            return game_id; 
        }
    }
    return -1;
}

void game_loop(int sd, int game_id) {
    int row, col, command;
    char buffer[4096]; 

    printf("\n--- PARTITA INIZIATA (ID: %d) ---\n", game_id);

    while(1) {
        // 1. Ricezione comando
        if (recv(sd, &command, sizeof(int), 0) <= 0) break;

        // 2. Ricezione tabellone/messaggio
        memset(buffer, 0, sizeof(buffer));
        if (recv(sd, buffer, sizeof(buffer)-1, 0) <= 0) break;

        printf("%s\n", buffer);

        if (command == CMD_OVER) {
            printf("\n--- PARTITA CONCLUSA ---\n");

            // Se hai perso (controlla il messaggio specifico definito nel server)
            if (strstr(buffer, "PAR O' CAZZ")) {
                printf("Hai perso. Premi INVIO per tornare al menu...");
                getchar(); clear_input();
                return; 
            }

            // GESTIONE RIVINCITA (Vittoria o Pareggio)
            // Controlla se la richiesta di rematch è arrivata attaccata al messaggio precedente
            // o se deve essere letta ora.
            if (!strstr(buffer, "REMATCH_REQUEST")) {
                memset(buffer, 0, sizeof(buffer));
                // Legge la stringa pura inviata da ask_rematch
                recv(sd, buffer, sizeof(buffer)-1, 0);
            }

            if (strstr(buffer, "REMATCH_REQUEST")) {
                int choice, action = REMATCH;
                printf("\n>>> VUOI UNA RIVINCITA? (1 = SI, 0 = NO): ");
                if (scanf("%d", &choice) != 1) choice = 0;
                clear_input();

                send(sd, &action, sizeof(int), 0);
                send(sd, &game_id, sizeof(int), 0);
                send(sd, &choice, sizeof(int), 0);

                if (choice == 1) {
                    printf("Hai accettato. In attesa...\n");

                    // Caso Pareggio: aspettiamo START o CMD_OVER (se l'altro rifiuta)
                    // Caso Vittoria: il server ci resetta e ci mette in wait (come create)
                    
                    // Proviamo a leggere cosa succede
                    memset(buffer, 0, sizeof(buffer));
                    int n = recv(sd, buffer, sizeof(buffer)-1, 0);

                    // Se arriva un comando numerico CMD_OVER (l'altro ha rifiutato)
                    // Nota: recv legge byte. Se il server manda un int, i primi 4 byte sono l'int.
                    // Qui facciamo una lettura "generica" per capire.
                    
                    if (n == 4) { // Probabile comando int (es. l'altro ha rifiutato)
                        int cmd_check;
                        memcpy(&cmd_check, buffer, 4);
                        if (cmd_check == CMD_OVER) {
                            printf("L'avversario ha rifiutato la rivincita.\n");
                            return;
                        }
                    }

                    if (n > 0) {
                        if (strstr(buffer, "START_PLAYER")) {
                            printf("Rivincita iniziata! Si riparte.\n");
                            continue; // Riparte il loop
                        } 
                        // Se siamo il vincitore, il server ci ha messo in wait per JOIN_REQUEST
                        // ma attenzione: ask_rematch chiama clear_game e poi wait.
                        // Il client deve gestire l'attesa.
                        // Dato che il protocollo del server è complesso (vincitore diventa host),
                        // la soluzione più semplice lato client per il vincitore è tornare in wait_for_challenger
                        // SE il server non manda subito START.
                        
                        // Per semplicità nel codice server attuale:
                        // Se pareggio -> Entrambi ricevono START
                        // Se vittoria -> Vincitore diventa Host e aspetta (JOIN_REQUEST).
                    }
                    
                    // Se non abbiamo ricevuto START immediato, probabilmente siamo in attesa (Vincitore)
                    int new_id = wait_for_challenger(sd);
                    if (new_id != -1) {
                         game_id = new_id;
                         continue;
                    }
                }
                return;
            }
            return;
        }
        else if (command == CMD_PLAY || command == CMD_INVALID) {
            if (command == CMD_INVALID) printf("[!] MOSSA NON VALIDA! ");
            
            printf("Tocca a te (riga colonna): ");
            if (scanf("%d %d", &row, &col) != 2) {
                clear_input();
            } else {
                send_move(sd, game_id, row, col);
            }
        }
        else if (command == CMD_WAIT) {
            printf("In attesa dell'avversario...\n");
        }
    }
}

void print_menu() {
    printf("\n========= TRIS MENU ========\n");
    printf("1. Crea nuova partita\n");
    printf("2. Lista partite attive\n");
    printf("3. Unisciti a una partita\n");
    printf("0. Esci\n");
    printf("============================\n");
    printf("Scelta: ");
}

int main() {
    int sd;
    struct sockaddr_in server_addr;
    int scelta;
    char buffer[4096];

    sd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(sd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connessione fallita");
        return 1;
    }
    printf("Connesso al server.\n");

    while (1) {
        print_menu();
        if (scanf("%d", &scelta) != 1) {
            clear_input();
            continue;
        }
        if (scelta == EXIT) break;
        send(sd, &scelta, sizeof(int), 0);

        if (scelta == CREATE) {
            int started_game_id = wait_for_challenger(sd);
            if (started_game_id != -1) game_loop(sd, started_game_id);
        } 
        else if (scelta == LIST) {
            memset(buffer, 0, sizeof(buffer));
            recv(sd, buffer, sizeof(buffer) - 1, 0);
            printf("\n%s", buffer);
        } 
        else if (scelta == JOIN) {
            int game_id;
            printf("Inserisci ID Partita: ");
            scanf("%d", &game_id);
            send(sd, &game_id, sizeof(int), 0);
            printf("In attesa...\n");
            
            memset(buffer, 0, sizeof(buffer));
            if (recv(sd, buffer, sizeof(buffer) - 1, 0) > 0 && strstr(buffer, "JOIN_OK")) {
                game_loop(sd, game_id);
            } else {
                printf("Rifiutato o errore.\n");
            }
        }
    }
    close(sd);
    return 0;
}