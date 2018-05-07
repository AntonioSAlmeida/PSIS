#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>

#include "library.h"


int main(){

	struct sockaddr_un server_addr;
	struct sockaddr_un client_addr;
	char buff[100];
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

	//while(1){
	//	printf("%s\n", "Enter your message");
	//}

	char * m=malloc(1000*sizeof(char));

	strcpy(m, "Message in a bottle\n");


	while(1){
		//Send to clipboard something
		clipboard_copy(sock_fd, 3, m, (strlen(m)+1)*sizeof(char));	
		sleep(3);
	}
	



	close(sock_fd);
	exit(0);
}
