#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "library.h"

char **clipboard_content;


void * clipboard_thread_connection(void * fdi){
		int * fd = fdi;
		int client_fd=*fd;
		char buff[100];
		int nbytes=1;
		int count=0;
		message *message_size=malloc(sizeof(message));
		printf("%lu\n", pthread_self());

		// Receive messages - 1st) size of message; 2nd) message to copy to clipboard
		while(1){
			nbytes = recv(client_fd, buff, sizeof(message), 0);
			if(nbytes==0){
				break;
			}
			//Build struct
			memcpy(message_size, buff, sizeof(message));

			printf("%d\n", message_size->data_size);

			count=message_size->data_size;
			while(count>0){
				nbytes = recv(client_fd, buff, message_size->data_size, 0);
				count-=nbytes;	
			}
			
			// DELETE - print to check variables
			printf("%s", buff);
			//Escrever no array dinamico
			clipboard_content[message_size->region] = malloc(message_size->data_size*sizeof(char));
			memcpy(clipboard_content[message_size->region], buff, message_size->data_size);
		}
		printf("leaving\n");
}

int main(int argc, char *argv[]){
	struct sockaddr_un server_addr;
	struct sockaddr_un client_addr;
	socklen_t size_addr;
		
	printf("%lu\n", pthread_self());
	
	int sock_fd;
	int remote_fd = -1;
	int client_fd;
	int* c_fd = malloc(sizeof(int));
	clipboard_content = malloc(10*sizeof(char*));

	pthread_t thread_ids[10];
	

	/*if(argc > 1){
		if(!strcmp(argv[1], "-c")){
			printf("in connected mode\n");
			struct sockaddr_in remote_addr;

			// Create connection to socket to communicate with remote clipboard
			remote_fd = socket(AF_INET, SOCK_STREAM, 0);

			if (remote_fd == -1){
				perror("socket: ");
				exit(-1);
			}

			remote_addr.sin_family = AF_INET;
			remote_addr.sin_port = htons(REMOTE_PORT);
			inet_aton(argv[2], &remote_addr.sin_addr);

			if(-1 == connect(remote_fd, (const struct sockaddr *) &remote_addr, sizeof(remote_addr))){
				printf("Error connecting\n");
				exit(-1);
			}

			printf("connected\n");
			// copy all data in current clipboard to remote clipboard
			clipboard_copy(remote_fd, 3, "I send an SOS to the world\n", (strlen("I send an SOS to the world\n")+1)*sizeof(char));
		}
	}*/

	// Create unix domain socket-stream socket
	sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock_fd == -1){
		perror("socket: ");
		exit(-1);
	}

	// Fill server_addr
	server_addr.sun_family = AF_UNIX;
	strcpy(server_addr.sun_path, CLIPBOARD_SOCKET);

	//Unlink & Bind socket to address server_addr
	unlink(CLIPBOARD_SOCKET);
	int err = bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if(err == -1) {
		perror("bind");
		exit(-1);
	}

	printf("-socket created and binded \n");

	// Listen
	listen(sock_fd, 5);
	printf("Ready to accept connections\n");

	// TO DO - Make a way to end the loop. Maybe an order "2" that's sent from server?
	int i=0;
	while(1){
		// Accept
		client_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &size_addr);
		printf("Accepted one connection from %s\n", CLIPBOARD_SOCKET);

		//Call new thread
		*c_fd=client_fd;
		pthread_create(&(thread_ids[i]), NULL, clipboard_thread_connection, c_fd);
		i++;
		
		
		/*if(remote_fd != -1){
			clipboard_copy(remote_fd, 3, "I send an SOS to the world\n", (strlen("I send an SOS to the world\n")+1)*sizeof(char));
		}*/
		printf("Thread creation successful\n");

		// TO DO - Create local clipboard
		// TO DO - Copy message to local clipboard
	}

	close(sock_fd);
	close(client_fd);

	exit(0);
}
