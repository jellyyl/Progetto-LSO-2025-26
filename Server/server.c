#include "server.h"


int start_server(int port, int backlog) {


    int sd;
    struct sockaddr_in server_addr;

    //creazione socket
    sd = socket(PF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);


    //assegnazione indirizzo locale al socket
    bind(sd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    
    //definito il numero massimo di connessioni pendenti in coda
    listen(sd, backlog);
    printf("Server in ascolto sulla porta %d...\n", port);

    return sd;

}


int accept_client(int sd) {
    struct sockaddr_in client_addr;
    socklen_t size_addr_client = sizeof(client_addr);

    // La accept si blocca finché un client non si connette
    int newsd = accept(sd, (struct sockaddr *) &client_addr, &size_addr_client);

    if(newsd < 0) {
        perror("Errore accept");
        return -1;
    }

    printf("Client connesso: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    return newsd;
}


int handle_client(int newsd, void *(*handler)(void *)) {

    int* new_sd_ptr;
    pthread_t tid;

    /*
        salvo il socket descriptor della connessione accettata, 
        nella memoria heap in modo che sia accessibile al thread creato
    */
    new_sd_ptr = malloc(sizeof(int));
    *new_sd_ptr = newsd;

    // Genera un thread per servire la richiesta 
    if (pthread_create(&tid, NULL, handler, (void *)new_sd_ptr) != 0) {
        perror("Errore creazione thread");
        close(newsd);
        free(new_sd_ptr);
        return -1;
    }
    
    // Il thread viene "staccato" per liberare risorse alla fine
    pthread_detach(tid);
    
    return 0;
}


void close_server(int sd) {
    close(sd);
}










