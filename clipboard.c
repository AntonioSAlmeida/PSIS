#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h> 
#include <unistd.h>
#include <sys/socket.h>
#include "clipboard.h"


int main(){
	struct sockaddr_un server_addr;
	struct sockaddr_un client_addr;
	
	socklen_t size_addr;

	char buff[100];
	int nbytes;
	
	//Socket
	int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock_fd == -1){
		perror("socket: ");
		exit(-1);
	}
	
	//Fill server_addr
	server_addr.sun_family = AF_UNIX;
	strcpy(server_addr.sun_path, SOCK_ADDRESS);

	//bind
	int err = bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if(err == -1) {
		perror("bind");
		exit(-1);
	}

	printf(" socket created and binded \n");

	//listen
	listen(sock_fd, 5);
	
	printf("Ready to accept connections\n");


	//ciclo
	//accept

	int client_fd= accept(sock_fd, (struct sockaddr *) & client_addr, &size_addr);

	printf("Accepted one connection from %s \n", (char*)client_addr.sun_path);

	message * message_received=NULL;
	message_received=malloc(sizeof(message));
	
	message * message_size=NULL;

	

	nbytes = recv(client_fd, buff, sizeof(message), 0);

	memcpy(message_size, buff, sizeof(message));

	nbytes = recv(client_fd, buff, message_size->data_size, 0);


	printf("received %d bytes --- %s ---\n", nbytes, buff);

	free(message_received);

	close(sock_fd);
	close(client_fd);

	exit(0);

}
