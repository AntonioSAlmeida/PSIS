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

	char* m = malloc(100*sizeof(char));
	char* region = malloc(100*sizeof(char));
	int reg = 0;
	void* buff = malloc(100*sizeof(char));

	//Send to clipboard something
	printf("What do you want to copy to the clipboard?\n-> ");
	fgets(m, 100*sizeof(char), stdin);
	printf("To what region do you want to copy?\n-> ");
	fgets(region, 100*sizeof(char), stdin);
	reg = atoi(region);
	clipboard_copy(sock_fd, reg, m, (strlen(m)+1)*sizeof(char));

    nbytes=0;
	nbytes = clipboard_paste(sock_fd, 3, (char *)buff, 5);
	printf("[PASTED] %s\n", (char*)buff);

	//wait until position 3 changes
	printf("waiting\n");
	clipboard_wait(sock_fd, 3, buff, 5);

	printf("WAITED ALL THIS TIME FOR THIS!?!?\n->%s\n", (char*)buff);

	// while(nbytes!=-1){
	// 	printf("From what region do you want to paste?\n-> ");
	// 	fgets(region, 100*sizeof(char), stdin);
	// 	reg = atoi(region);
	// 	nbytes = clipboard_paste(sock_fd, reg, (char *)buff, 5);
	// 	sleep(2);
	// }

	close(sock_fd);
	free(m);
	free(buff);
	exit(0);
}
