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
	char buff[100];
	int nbytes;
	int sock_fd=0;
	
	printf(" socket created \n");
	client_addr.sun_family = AF_UNIX;
	printf(" socket with adress \n");
	server_addr.sun_family = AF_UNIX;
	strcpy(server_addr.sun_path, SOCK_ADDRESS);

	sock_fd=clipboard_connect(server_addr);

	clipboard_copy(sock_fd, 3, "penis\n", (strlen("penis\n")+1)*sizeof(char));

	

	close(sock_fd);
	exit(0);
}
