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

	//C0nnect
	sock_fd = clipboard_connect(server_addr);

	char* m = malloc(100*sizeof(char));
	char* region = malloc(100*sizeof(char));
	char* action = malloc(100*sizeof(char));
	int order = 0;
	int reg = 0;
	int count = 0;
	void* buff = malloc(100*sizeof(char));
	char* buff2 = malloc(100*sizeof(char));


	//User input
	printf("Write something to copy to some regions\n->");
	fgets(m, 100*sizeof(char), stdin);
	printf("To what region do you want to copy?\n->");
	fgets(buff2, 100*sizeof(char), stdin);
	reg = atoi(buff2);

	//other orders order
	while(1){
		printf("What action do you want to execute?\n->");
		fgets(action, 100*sizeof(char), stdin);

		if(strcmp(action, "COPY") == 0){
			printf("Write something to copy to some regions\n->");
			fgets(m, 100*sizeof(char), stdin);
			printf("To what region do you want to copy?\n->");
			fgets(buff2, 100*sizeof(char), stdin);
			reg = atoi(buff2);
			printf("How many bytes do you want to copy?\n->");
			fgets(buff2, 100*sizeof(char), stdin);
			count = atoi(buff2);
			n_bytes = clipboard_copy(sock_fd, reg, buff, count);

		}else if(strcmp(action, "PASTE") == 0){
			printf("From what region do you want to paste?\n->");
			fgets(buff2, 100*sizeof(char), stdin);
			reg = atoi(buff2);
			printf("How many bytes do you want to paste?\n->");
			fgets(buff2, 100*sizeof(char), stdin);
			count = atoi(buff2);
			n_bytes = clipboard_paste(sock_fd, reg, buff, count);

		}else if(strcmp(action, "WAIT") == 0){
			printf("For what region do you want to wait?\n->");
			fgets(buff2, 100*sizeof(char), stdin);
			reg = atoi(buff2);
			printf("For many bytes do you want to wait?\n->");
			fgets(buff2, 100*sizeof(char), stdin);
			count = atoi(buff2);
			n_bytes = clipboard_copy(sock_fd, reg, buff, count);

		}

	}

	close(sock_fd);
	free(m);
	free(buff);
	free(buff2);
	exit(0);

}
