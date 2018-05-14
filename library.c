#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h> 
#include <unistd.h>
#include "library.h"


/********************************************************************************
 * Function called by the application to connect to a distributed clipboard.	*
 *																				*
 *char *clipboard directory - directory where the clipboard was created.		*
 *																				*
 *output - file descriptor of the socket created between the application and	*
 *			the clipboard. 														*
 *																				*
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
 * Copies data pointed by buf to a region on the local clipboard.				*
 *																				*
 * char *clipboard directory - directory where the clipboard was created.		*
 *																				*
 * output - size of data copied to buf.											*
 *			0 if error.															*
 *																				*
 ********************************************************************************/
int clipboard_copy(int clipboard_id, int region, void *buf, size_t count){

	if(region >= 10){
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
	}
	
	free(msg);
	
	return count;
}

/********************************************************************************
 * Copies data from a region on the local clipboard to memory pointed by buf.	*
 *																				*
 * char *clipboard directory - directory where the clipboard was created.		*
 *																				*
 * output - file descriptor of the socket created between the application and	*
 *			the clipboard. 														*
 *			0 if error.															*
 *																				*
 ********************************************************************************/
int clipboard_paste(int clipboard_id, int region, void *buf, size_t count){

	if(region >= 10){
		return 0;
	}

	char *bufstruct=malloc(sizeof(message));
	message msg_struct;
	msg_struct.order = PASTE;
	msg_struct.region = region;
	msg_struct.data_size=0;
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

	//Receive messsage
	buf = malloc(msg_struct.data_size);

	err_msg = recv(clipboard_id, buf, msg_struct.data_size, 0);
	if(err_msg == -1){
		perror("clipboard: ");
		return 0;
	}

	//printf("%s\n", (char*)buf);

	free(bufstruct);
	free(msg);
	return count;
}

/********************************************************************************
 * Waits for a change in a certain region and, when it happens, copies the new	*
 * data	to memory pointed by buf.												*
 *																				*
 * char *clipboard directory - directory where the clipboard was created.		*
 *																				*
 * output - number of bytes copied to buf.										*
 *			0 if error.															*
 *																				*
 ********************************************************************************/
int clipboard_wait(int clipboard_id, int region, void *buf, size_t count){

	char *init_clip = (char*)malloc((sizeof(char)*count));
	char *current_clip = (char*)malloc((sizeof(char)*count));

	int err = clipboard_paste(clipboard_id, region, init_clip, count);
	if(err == -1){
		perror("paste: ");
		return 0;
	}

	current_clip = init_clip;

	while(strcmp(init_clip, current_clip)==0){
		sleep(2); //just to try and not consume too many resources by constantly making comparisons...
		int err = clipboard_paste(clipboard_id, region, current_clip, count);
		if(err == 0){
			perror("paste: ");
			return 0;
		}
	}

	buf = malloc((sizeof(char)*count)+1);
	buf = current_clip;

	free(init_clip);
	free(current_clip);
	return count;
}

/********************************************************************************
 * Closes the connection between the application and the local clipboard.		*
 *																				*
 * int clipboard_id - file descriptor of the connection to the local clipboard.	*
 *																				*
 ********************************************************************************/
void clipboard_close(int clipboard_id){
	close(clipboard_id);
}