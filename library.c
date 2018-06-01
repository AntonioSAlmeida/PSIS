#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "LinkedList.h"
#include "clipboard.h"
#include "message.h"

/********************************************************************************
 * Function called by the application to connect to a distributed clipboard.
 *
 * char *clipboard directory - directory where the clipboard was created.
 *
 * output - file descriptor of the socket created between the application and
 *					the clipboard.
 *
 ********************************************************************************/
int clipboard_connect(struct sockaddr_un server_addr){

	int sock_fd= socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock_fd == -1){
		perror("socket: ");
		return -1;
	}

	if(connect(sock_fd, (const struct sockaddr *) &server_addr, sizeof(server_addr))==-1){
		printf("Error connecting\n");
		return -1;
	}

	return sock_fd;
}

/********************************************************************************
 * Copies data pointed by buf to a region on the local clipboard.
 *
 * char *clipboard directory - directory where the clipboard was created.
 *
 * output - size of data copied to buf.
 *					0 if error.
 *
 ********************************************************************************/
int clipboard_copy(int clipboard_id, int region, void *buf, size_t count){

	if(region >= 10){
		perror("invalid region: ");
		return 0;
	}

	message msg_struct;
	msg_struct.order = COPY;
	msg_struct.region = region;
	msg_struct.data_size = count;
	int aux_count=count;
	int sent=0;
	char *msg = malloc(sizeof(message));

	memcpy(msg, &msg_struct, sizeof(message));

	sent = send(clipboard_id, msg, sizeof(message), 0);

	if(sent == -1){
		perror("clipboard: ");
		return 0;
	}

	while(aux_count > 0){
		sent = send(clipboard_id, buf, msg_struct.data_size, 0);
		if(sent == -1){
			perror("clipboard: ");
			return 0;
		}
		//if(aux_count!=count){
		//	printf("repeating send\n");
		//}

		aux_count-=sent;
		//printf("%d\n", aux_count);
	}

	//printf("came out\n");
	free(msg);

	return count;
}

/********************************************************************************
 * Copies data from a region on the local clipboard to memory pointed by buf.
 *
 * char *clipboard directory - directory where the clipboard was created.				*
 *
 * output - file descriptor of the socket created between the application and
 *					the clipboard.
 *					0 if error.
 *
 ********************************************************************************/
int clipboard_paste(int clipboard_id, int region, void *buf, size_t count){

	if(region >= 10){
		return 0;
	}

	message msg_struct;
	msg_struct.order = PASTE;
	msg_struct.region = region;
	msg_struct.data_size = count;
	char *bufstruct=malloc(sizeof(message));
	int err_msg;
	char *msg = malloc(sizeof(message));


	memcpy(msg, &msg_struct, sizeof(message));
	err_msg = send(clipboard_id, msg, sizeof(msg), 0);
	if(err_msg == -1){
		perror("clipboard: ");
		return 0;
	}

	//Receive message struct
	err_msg = recv(clipboard_id, bufstruct, sizeof(message), 0);
	if(err_msg == -1){
		perror("clipboard: ");
		return 0;
	}

	memcpy(&msg_struct, bufstruct, sizeof(message));


	err_msg = recv(clipboard_id, buf, msg_struct.data_size, 0);
	if(err_msg == -1){
		perror("clipboard: ");
		return 0;
	}

	free(bufstruct);
	free(msg);
	return strlen(buf)+1;
}

/********************************************************************************
 * Waits for a change in a certain region and, when it happens, copies the new	*
 * data	to memory pointed by buf.
 *
 * char *clipboard directory - directory where the clipboard was created.
 *
 * output - number of bytes copied to buf.
 *					0 if error.
 *
 ********************************************************************************/
int clipboard_wait(int clipboard_id, int region, void *buf, size_t count){

	if(region >= 10){
		return 0;
	}

	message msg_struct;
	msg_struct.order = WAIT;
	msg_struct.region = region;
	msg_struct.data_size = count;

	char* bufstruct = malloc(sizeof(message));
	int err_msg;
	char* msg = malloc(sizeof(message));


	memcpy(msg, &msg_struct, sizeof(message));
	err_msg = send(clipboard_id, msg, sizeof(msg), 0);
	if(err_msg == -1){
		perror("clipboard: ");
		return 0;
	}

	//Receive message struct
	err_msg = recv(clipboard_id, bufstruct, sizeof(message), 0);
	if(err_msg == -1){
		perror("clipboard: ");
		return 0;
	}

	memcpy(&msg_struct, bufstruct, sizeof(message));

	//Write count bytes from the received message in buf, if size of message is larger or equal to count.
	//Else, write the whole message.
	if(count > msg_struct.data_size){
		err_msg = recv(clipboard_id, buf, msg_struct.data_size, 0);
		if(err_msg == -1){
			perror("clipboard: ");
			return 0;
		}

		free(bufstruct);
		free(msg);
		return msg_struct.data_size;
	}else{
		err_msg = recv(clipboard_id, buf, count, 0);
		if(err_msg == -1){
			perror("clipboard: ");
			return 0;
		}

		free(bufstruct);
		free(msg);
		return count;
	}

	return 0;
}

/*******************************************************************************
 * Closes the connection between the application and the local clipboard.
 *
 * int clipboard_id - file descriptor of the connection to the local clipboard.	*
 *
 ********************************************************************************/
void clipboard_close(int clipboard_id){
	close(clipboard_id);
}
