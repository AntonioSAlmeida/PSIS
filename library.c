#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h> 
#include <unistd.h>
#include "clipboard.h"

#define COPY 0
#define PASTE 1


int clipboard_connect(struct sockaddr_un server_addr){

	int sock_fd= socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock_fd == -1){
		perror("socket: ");
		exit(-1);
	}
	
	if( -1 == connect(sock_fd, (const struct sockaddr *) &server_addr, sizeof(server_addr))){
		printf("Error connecting\n");
		exit(-1);
	}

	return sock_fd;
}

int clipboard_copy(int clipboard_id, int region, void *buf, size_t count){

	message msg_struct;
	
	msg_struct.order = COPY;
	msg_struct.region = region;
	msg_struct.data_size = sizeof(char)*(strlen(buf)+1);

	//char *msg = malloc(sizeof(message));

	//memcpy(msg, &msg_struct, sizeof(msg_struct));

	send(clipboard_id, &msg_struct, sizeof(msg_struct), 0);

	send(clipboard_id, buf, msg_struct.data_size, 0);
	
	
	return 0;
}

int clipboard_paste(int clipboard_id, int region, void *buf, size_t count){

	message msg_struct;
	msg_struct.order = PASTE;
	msg_struct.region = region;
	msg_struct.data_size=0;
	char *msg = malloc(sizeof(message));

	memcpy(msg, &msg_struct, sizeof(message));

	send(clipboard_id, msg, sizeof(msg), 0);

	recv(clipboard_id+1, &buf, 100, 0);
	
	free(msg);
	return 0;
}
