
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "server.h"


// Funzione eseguita dal thread per gestire il client
void *gestisci_client(void *arg) {
    int newsd = *(int *)arg; // Recupera il socket descriptor
    free(arg);               // Libera la memoria allocata nel main
    int numero, doppio;

    // Legge il messaggio del client dalla socket [cite: 906]
    if (read(newsd, &numero, sizeof(int)) > 0) {
        printf("[Thread %ld] Ricevuto: %d\n", pthread_self(), numero);
        doppio = numero * 2;
        
        // Invia il risultato al client
        write(newsd, &doppio, sizeof(int));
    }

    close(newsd); // Chiude la connessione con questo client
    pthread_exit(NULL); // Termina il thread
}

int main() {

    int sd;
    sd = start_server(5200, 5);


    while(1) {
        int newsd = accept_client(sd);

        handle_client(newsd, gestisci_client);
    }


    close_server(sd);
    return 0;

}
