#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

// Definizioni coerenti con Actions in game.h
#define CREATE 1
#define LIST 2
#define EXIT 0

#define SERVER_IP "127.0.0.1"
#define PORT 5200

void print_menu() {
    printf("\n--- MENU TRIS INTERATTIVO ---\n");
    printf("1. Crea una nuova partita (CREATE)\n");
    printf("2. Visualizza lista partite (LIST)\n");
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
            printf("Comando CREATE inviato. Controlla con LIST se la partita appare.\n");
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
        } 
        else {
            printf("Azione non valida.\n");
        }
    }

    close(sd);
    printf("Client chiuso.\n");
    return 0;
}