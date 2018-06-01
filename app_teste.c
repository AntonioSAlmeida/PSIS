#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>

#include "clipboard.h"


int main(){

	struct sockaddr_un server_addr;
	struct sockaddr_un client_addr;
	int nbytes;
	int sock_fd=0;



	//Create and fill client address
	printf(" socket created \n");
	client_addr.sun_family = AF_UNIX;

	//Create and fill clipboard server address
	printf(" socket with adress \n");
	server_addr.sun_family = AF_UNIX;
	strcpy(server_addr.sun_path, CLIPBOARD_SOCKET);

	//Create socket
	sock_fd=clipboard_connect(server_addr);

	char* m = malloc(100*sizeof(char));
	char* region = malloc(100*sizeof(char));
	int reg = 0;
	void* buff = malloc(100*sizeof(char));
	void* buff2 = malloc(100*sizeof(char));

	//Send to clipboard something
	printf("Write something to copy to some regions\n-> ");
	fgets(m, 100*sizeof(char), stdin);
	printf("Write something else to write to some regions\n->");
	fgets(buff2, 100*sizeof(char), stdin);

	fork();
	fork();
	fork();
	

	srand(getpid());
	reg = (rand() % 9);
	if(getpid()%2==0)
		clipboard_copy(sock_fd, reg, m, (strlen(m)+1)*sizeof(char));
	else
		clipboard_copy(sock_fd, reg, buff2, (strlen(buff2)+1)*sizeof(char));

	if(getpid()==0){
		//wait until position 3 changes
		printf("waiting\n");
		clipboard_wait(sock_fd, 3, buff, 100);
		printf("WAITED ALL THIS TIME FOR THIS!?!?\n->%s\n", (char*)buff);
	
		close(sock_fd);
		exit(0);
	}

	free(m);
	free(buff);
	
}
