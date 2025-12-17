#ifndef SERVER_HEADER_H
#define SERVER_HEADER_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BACKLOG 5



int start_server(int port, int backlog);
int accept_client(int sd);
int handle_client(int newsd, void *(*handler)(void *));
void close_server(int sd);



#endif // SERVER_HEADER_H