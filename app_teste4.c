#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <signal.h>
#include <pthread.h>
#include "clipboard.h"


char* m;
char* region;
char* action;	
char* buff;
char* buff2;
int sock_fd=0;

void sig_handler(int dummy)
{
	printf("Clipboard Died\n");
	free(m);
	free(action);
	free(buff2);
	close(sock_fd);																	
	exit(0);
}


int main(){

	m = malloc(100*sizeof(char));
	region = malloc(100*sizeof(char));
	action = malloc(100*sizeof(char));	
	buff = malloc(100*sizeof(char));
	buff2 = malloc(100*sizeof(char));

	struct sockaddr_un server_addr;
	struct sockaddr_un client_addr;
	int n_bytes;
	//Create and fill client address
	printf(" socket created \n");
	client_addr.sun_family = AF_UNIX;

	//Create and fill clipboard server address
	printf(" socket with adress \n");
	server_addr.sun_family = AF_UNIX;
	strcpy(server_addr.sun_path, CLIPBOARD_SOCKET);

	//Connect
	sock_fd = clipboard_connect(server_addr);

	int order = 0;
	int reg = 0;
	int count = 0;
	n_bytes=0;
	


	//User input
	signal(SIGINT, sig_handler);


	//other orders order
	while(1){
		action = malloc(100*sizeof(char));	
		m = malloc(100*sizeof(char));
		buff2 = malloc(100*sizeof(char));
		
		
		printf("What action do you want to execute?\n->");
		fgets(action, 100*sizeof(char), stdin);

		if(!strcasecmp(action, "COPY\n")){
			printf("Write something to copy to some regions\n->");
			fgets(m, 100*sizeof(char), stdin);
			printf("To what region do you want to copy?\n->");
			fgets(buff2, 100*sizeof(char), stdin);

			reg = atoi(buff2);
		
			count = strlen(m)+1;
			n_bytes = clipboard_copy(sock_fd, reg, m, count);
			printf("\nCopy\nsize:%d\nregion:%d\nmessage:%s\n", n_bytes, reg, m);

		}else if(strcasecmp(action, "PASTE\n") == 0){
			printf("From what region do you want to paste?\n->");
			fgets(buff2, 100*sizeof(char), stdin);
			reg = atoi(buff2);
		
			printf("How many bytes do you want to paste?\n->");
			fgets(buff2, 100*sizeof(char), stdin);
			count = atoi(buff2);
			m = realloc(m, count*sizeof(char));
			n_bytes = clipboard_paste(sock_fd, reg, m, count);
			printf("\nPaste\nsize:%d\nregion:%d\nmessage:%s\n", n_bytes, reg, m);

		}else if(strcasecmp(action, "WAIT\n") == 0){
			printf("For what region do you want to wait?\n->");
			fgets(buff2, 100*sizeof(char), stdin);
			reg = atoi(buff2);
		
			printf("For many bytes do you want to wait?\n->");
			fgets(buff2, 100*sizeof(char), stdin);
			count = atoi(buff2);
			m = realloc(m, (count)*sizeof(char));
			n_bytes = clipboard_wait(sock_fd, reg, m, count);
			printf("\nWaited\nsize:%d\nregion:%d\nmessage:%s\n", n_bytes, reg, m);

		}else{
			printf("Unknown Command: %s\n", action);
		}

		free(m);
		free(action);
		free(buff2);

	}

	close(sock_fd);
	free(m);
	free(action);
	free(buff2);
	exit(0);

}
