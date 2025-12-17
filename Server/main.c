
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


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

    int sd, *new_sd_ptr;
    struct sockaddr_in server_addr;
    pthread_t tid;

    //creazione socket
    sd = socket(PF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5200);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);


    //assegnazione indirizzo locale al socket
    bind(sd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    
    //definito il numero massimo di connessioni pendenti in coda
    listen(sd, 5);
    printf("Server in ascolto sulla porta 5200...\n");


    while(1) {
        struct sockaddr_in client_addr;
        socklen_t size_addr_client = sizeof(client_addr);

        // La accept si blocca finché un client non si connette
        int newsd = accept(sd, (struct sockaddr *) &client_addr, &size_addr_client);

        /*
         salvo il socket descriptor della connessione accettata, 
         nella memoria heap in modo che sia accessibile al thread creato
        */
        new_sd_ptr = malloc(sizeof(int));
        *new_sd_ptr = newsd;

        // Genera un thread per servire la richiesta 
        if (pthread_create(&tid, NULL, gestisci_client, (void *)new_sd_ptr) != 0) {
            perror("Errore creazione thread");
            close(newsd);
            free(new_sd_ptr);
        }
        
        // Il thread viene "staccato" per liberare risorse alla fine
        pthread_detach(tid);


    }


    close(sd);
    return 0;

}