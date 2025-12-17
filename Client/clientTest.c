#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main() {
    int sd, n, risposta;
    struct sockaddr_in srv;

    sd = socket(PF_INET, SOCK_STREAM, 0);
    srv.sin_family = AF_INET;
    srv.sin_port = htons(5200);
    inet_aton("127.0.0.1", &srv.sin_addr);

    if (connect(sd, (struct sockaddr *)&srv, sizeof(srv)) < 0) {
        perror("Connessione fallita"); exit(1);
    }

    printf("Inserisci un intero: ");
    scanf("%d", &n);

    write(sd, &n, sizeof(int)); // Invia al server
    read(sd, &risposta, sizeof(int)); // Riceve il doppio

    printf("Il doppio ricevuto dal server è: %d\n", risposta);

    close(sd);
    return 0;
}