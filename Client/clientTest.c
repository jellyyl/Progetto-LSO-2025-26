#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_IP "127.0.0.1"
#define PORT 5200

// --- PROTOCOLLO AZIONI ---
typedef enum {
    CREATE = 1,
    LIST = 2,
    JOIN = 3,
    MOVE = 4,
    REMATCH = 5,
    APPROVE = 6

} Action;

// --- PROTOCOLLO SERVER ---
#define CMD_PLAY 1
#define CMD_WAIT 2
#define CMD_OVER 3
#define CMD_INVALID 4
#define CMD_QUIT 5 //disconnesione utente da parita
#define CMD_QUIT_PRE_GAME 6 //disconnesione utente pre-partita

// Prototipi
void game_loop(int sd, int game_id);
void wait_for_challenger(int sd); // Nuova funzione per gestire l'attesa
void handle_creation(int sd);
void handle_join(int sd);
void handle_list(int sd);
int gestisci_rematch(int sd, int game_id, int is_draw, int *became_owner);

int main() {
    int sd;
    struct sockaddr_in server_addr;

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0) { perror("Socket"); exit(1); }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(sd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect"); exit(1);
    }

    printf("--- CLIENT TRIS CONNESSO ---\n");

    while (1) {
        printf("\n--- MENU ---\n1. Crea partita\n2. Lista partite\n3. Unisciti\n0. Esci\nScelta: ");
        int choice;
        if (scanf("%d", &choice) != 1) break;

        switch (choice) {
            case 1: handle_creation(sd); break;
            case 2: handle_list(sd); break;
            case 3: handle_join(sd); break;
            case 0: close(sd); exit(0);
            default: printf("Scelta non valida.\n");
        }
    }
    return 0;
}


// Funzione che gestisce l'attesa di un avversario (Usata in creazione e in rivincita da vincitore)
void wait_for_challenger(int sd) {
    char buffer[128];
    printf(">> In attesa di uno sfidante...\n");

    // Il server invia "JOIN_REQUEST" + game_id (17 bytes tot)
    memset(buffer, 0, sizeof(buffer));
    int bytes = recv(sd, buffer, 17, 0); 
    
    if (bytes <= 0) return;

    if (strncmp(buffer, "JOIN_REQUEST", 12) == 0) {
        int req_game_id;
        memcpy(&req_game_id, buffer + 13, sizeof(int));
        
        printf("\n[!] Un giocatore vuole unirsi alla partita %d.\n", req_game_id);
        printf("Accettare? (0: Si, 1: No): ");
        int response;
        scanf("%d", &response);

        // Invio APPROVE
        int act = APPROVE;
        send(sd, &act, sizeof(int), 0);
        send(sd, &req_game_id, sizeof(int), 0);
        send(sd, &response, sizeof(int), 0);

        if (response == 0) {
            // Consuma messaggio START_PLAYER1
            char start_msg[32];
            memset(start_msg, 0, sizeof(start_msg));
            recv(sd, start_msg, sizeof(start_msg), 0); // "START_PLAYER1"
            
            // Avvia il loop di gioco
            game_loop(sd, req_game_id);
        } else {
            printf("Hai rifiutato. Torni al menu.\n");
        }
    } else {
        printf("Messaggio imprevisto durante l'attesa: %s\n", buffer);
    }
}

void game_loop(int sd, int game_id) {
    int cmd;
    char buffer[2048];

    printf("\n--- PARTITA INIZIATA (ID: %d) ---\n", game_id);

    while (1) {
        // Legge comando intero (4 bytes)
        if (recv(sd, &cmd, sizeof(int), 0) <= 0) break;

        memset(buffer, 0, sizeof(buffer));

        switch (cmd) {
            case CMD_PLAY:
                recv(sd, buffer, sizeof(buffer) - 1, 0); // Legge tabellone
                printf("\n%s\n>> IL TUO TURNO (riga colonna): ", buffer);
                int r, c;
                scanf("%d %d", &r, &c);
                int action = MOVE;
                send(sd, &action, sizeof(int), 0);
                send(sd, &game_id, sizeof(int), 0);
                send(sd, &r, sizeof(int), 0);
                send(sd, &c, sizeof(int), 0);
                break;

            case CMD_WAIT:
                recv(sd, buffer, sizeof(buffer) - 1, 0); // Legge tabellone
                printf("\n%s\n>> In attesa dell'avversario...\n", buffer);
                break;

            case CMD_INVALID:
                printf(">> Mossa non valida!\n");
                break;

            case CMD_OVER:
                recv(sd, buffer, sizeof(buffer) - 1, 0);
                printf("\n%s\n", buffer);

                int is_draw = (strstr(buffer, "PAREGGIO") != NULL);
                int is_loser = (strstr(buffer, "PAR O' CAZZ") != NULL);

                if (is_loser) {
                    printf(">>> HAI PERSO. Game Over.\n");
                    return; 
                }

                // Gestione Rematch per Vincitore o Pareggio
                int became_owner = 0;
                int rematch_ok = gestisci_rematch(sd, game_id, is_draw, &became_owner);

                if (rematch_ok == 1) {
                    if (is_draw) {
                        continue;
                    } else {
                        // Se ho vinto (o sono diventato proprietario), devo attendere un nuovo sfidante
                        // Esco da game_loop ed entro in wait_for_challenger
                        printf(">>> In attesa di un nuovo sfidante nel server...\n");
                        wait_for_challenger(sd);
                        return; // Esco da questa istanza di game_loop, ne verrà lanciata una nuova da wait_for_challenger
                    }
                } else {
                    printf(">>> Partita terminata.\n");
                    return;
                }
                break;
            case CMD_QUIT:
                printf("\n------------------------------------------------\n");
                printf("   L'AVVERSARIO SI È CAGATO SOTTO!  \n");
                printf(" Hai vinto a tavolino per abbandono.\n");
                printf("------------------------------------------------\n");
                return;
                break; // Esci dal ciclo di gioco e torna al menu
            default:
                // Se arriva spazzatura, puliamo il buffer per evitare loop infiniti
                printf("Comando ignoto (%d). Pulizia buffer...\n", cmd);
                recv(sd, buffer, sizeof(buffer), MSG_DONTWAIT);
                break;
        }
    }
}

// Ritorna 1 se si continua, 0 se si esce
int gestisci_rematch(int sd, int game_id, int is_draw, int *became_owner) {
    char msg[128];
    memset(msg, 0, sizeof(msg));

    // 1. Controlla messaggi preliminari (Vittoria/Cambio Proprietario)
    // Usiamo MSG_DONTWAIT o un timeout se volessimo essere non bloccanti, 
    // ma qui ci aspettiamo messaggi precisi.
    if (recv(sd, msg, sizeof(msg) - 1, 0) <= 0) return 0;

    // Se eri ospite e hai vinto, il server ti notifica il cambio ruolo
    if (strstr(msg, "CHANGE_OWNER") != NULL) {
        printf("[INFO] Sei diventato il PROPRIETARIO della partita!\n");
        *became_owner = 1;
        // Pulisci e leggi il prossimo messaggio (che sarà REMATCH_REQUEST)
        memset(msg, 0, sizeof(msg));
        recv(sd, msg, sizeof(msg) - 1, 0);
    }

    // 2. Gestione Richiesta Utente
    if (strstr(msg, "REMATCH_REQUEST") != NULL) {
        int response;
        if (is_draw) printf("\n[PAREGGIO] Rigiocare con lo stesso avversario? (1:Si, 0:No): ");
        else printf("\n[VITTORIA] Vuoi sfidare un nuovo giocatore? (1:Si, 0:No): ");
        
        scanf("%d", &response);

        // Invia risposta al server
        int action = REMATCH;
        send(sd, &action, sizeof(int), 0);
        send(sd, &game_id, sizeof(int), 0);
        send(sd, &response, sizeof(int), 0);

        if (response == 0) {
            printf("Hai rifiutato la rivincita.\n");
            return 0; // Torna al menu
        }

        // 3. Attesa Risposta Server (Cruciale per il pareggio)
        // Se non è pareggio (è vittoria), il server ci mette in wait (se 1) o ci caccia (se 0)
        // Ma nel tuo server, se è vittoria, rematch_by_winner viene chiamato.
        if (!is_draw) return 1; 

        printf("In attesa della decisione dell'avversario...\n");

        char response_buf[64];
        memset(response_buf, 0, sizeof(response_buf));
        
        // Ora leggiamo semplicemente la risposta, che è SICURAMENTE una stringa
        int bytes = recv(sd, response_buf, sizeof(response_buf) - 1, 0);
        if (bytes <= 0) return 0;

        printf("Risposta avversario: %s\n", response_buf);

        // CASO A: Rifiuto
        if (strstr(response_buf, "REMATCH_DECLINED") != NULL) {
            printf(">>> La rivincita è stata DECLINATA.\n");
            return 0; // Torna al menu
        }

        // CASO B: Accettazione
        if (strstr(response_buf, "START_PLAYER") != NULL) {
            printf(">>> Entrambi hanno accettato! Riavvio partita...\n");
            // Nota: Abbiamo già "consumato" la stringa START_PLAYER leggendola in response_buf
            // Quindi possiamo tornare 1 e il loop leggerà pulito il prossimo CMD_PLAY/WAIT
            return 1; 
        }
    }
    
    return 0;
}

void handle_creation(int sd) {
    int action = CREATE;
    send(sd, &action, sizeof(int), 0);
    printf("Partita creata.\n");
    wait_for_challenger(sd); // Riutilizzo la logica di attesa
}

void handle_join(int sd) {
    int action = JOIN;
    printf("Inserisci ID partita: ");
    int game_id;
    scanf("%d", &game_id);

    send(sd, &action, sizeof(int), 0);
    send(sd, &game_id, sizeof(int), 0);

    char response[64];
    memset(response, 0, sizeof(response));
    recv(sd, response, sizeof(response) - 1, 0);

    if (strncmp(response, "JOIN_OK", 7) == 0) {
        printf("Accesso consentito!\n");
        // Aspettiamo START_PLAYER2 o WAIT
        // Il server in join manda CMD_WAIT direttamente in game_loop
        game_loop(sd, game_id);
    }
    else if (strncmp(response, "JOIN_ERR_OWNER_LEFT", 19) == 0){
        printf("L'avversario si è disconesso!\n");
        printf("Verrai riportato al menu\n");
        return;
    }
    else {
        printf("Errore: %s\n", response);
    }
}

void handle_list(int sd) {
    int action = LIST;
    send(sd, &action, sizeof(int), 0);
    char buffer[4096] = {0};
    recv(sd, buffer, sizeof(buffer) - 1, 0);
    printf("\n--- PARTITE ---\n%s\n", buffer);
}