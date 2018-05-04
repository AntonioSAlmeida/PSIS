#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "library.h"


int main(){
	struct sockaddr_in client_addr;
	socklen_t size_addr;
	char buff[100];
	char ** backup=malloc(10*sizeof(char*));
	int n_content=0;
	int nbytes;
	int sock_fd;
	int client_fd;
	message *message_size = (message *)malloc(sizeof(message));

	// Create unix domain socket-stream socket
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd == -1){
		perror("socket: ");
		exit(-1);
	}

	// Fill server_addr
	client_addr.sin_family=AF_INET;
	client_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	client_addr.sin_port=htons(REMOTE_PORT);


	// Bind socket to address server_addr
	int err = bind(sock_fd, (struct sockaddr *)&client_addr, sizeof(client_addr));
	if(err == -1) {
		perror("bind");
		exit(-1);
	}

	printf("-socket created and binded \n");

	// Listen
	listen(sock_fd, 5);
	printf("Ready to accept connections\n");

	// TO DO - Make a way to end the loop. Maybe an order "2" that's sent from server?
	while(1){
		// Accept
		client_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &size_addr);

		printf("Accepted one connection from %s\n", inet_ntoa(client_addr.sin_addr));

		// Receive messages - 1st) size of message; 2nd) message to copy to backup
		nbytes = recv(client_fd, buff, sizeof(message), 0);
		memcpy(message_size, buff, sizeof(message));
		printf("One less error to check\n");
		// DELETE - Print to check variables
		printf("\n%d | %d | %d\n\n", message_size->order, message_size->region, message_size->data_size);

		nbytes = recv(client_fd, buff, message_size->data_size, 0);
		// DELETE - print to check variables
		printf("\n%s\n\n", buff);

		if(backup[message_size->region]!=NULL){
			free(backup[message_size->region]);
		}

		backup[message_size->region]=malloc(message_size->data_size*sizeof(char));
		memcpy(backup[message_size->region], buff, message_size->data_size);


		// TO DO - Create local clipboard
		// TO DO - Copy message to local clipboard
	}

	close(sock_fd);
	close(client_fd);

	exit(0);
}
