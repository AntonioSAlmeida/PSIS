/* This is actuall API.c */


#include "clipboard.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define COPY "copy request"
#define PASTE "paste request"

typedef struct Message{
	char *order;
	int region;
	void *data;

} message;

/*fifo_send and fifo_recv are the file descriptors for the fifos to send and receive*/

int clipboard_connect(char *clipboard_dir){
	char fifo_name[100];

	if(sprintf(fifo_name, "%s%s", clipboard_dir, INBOUND_FIFO) < 0){
		printf("sprintf1 error");
		return -1;
	}

	int fifo_send = open(fifo_name, O_WRONLY);
	if(sprintf(fifo_name, "%s%s", clipboard_dir, OUTBOUND_FIFO) < 0){
		printf("sprintf2 error");
		exit(-1);
	}
	int fifo_recv = open(fifo_name, O_RDONLY);

	if(fifo_send < 0 || fifo_recv <0){
		printf("%d | %d", fifo_send, fifo_recv);
		exit(-1);
	}

	return fifo_send;
	//rcv is send+1
}

/* clipboard_id - file descriptor of the fifo to write
	 *buf 				- message to send
	 count 				- size of the sent message*/
int clipboard_copy(int clipboard_id, int region, void *buf, size_t count){

	struct Message msg_struct;
	msg_struct.order = COPY;
	msg_struct.region = region;
	msg_struct.data = buf;
	char *msg = malloc(sizeof(message));

	memcpy(msg, &msg_struct, sizeof(msg_struct));

	if(write(clipboard_id, &msg, count)<0){
		exit(-1);
	}

	free(msg);
	return 0;
}

int clipboard_paste(int clipboard_id, int region, void *buf, size_t count){

	message msg_struct;
	msg_struct.order = PASTE;
	msg_struct.region = region;
	msg_struct.data;
	char *msg = malloc(sizeof(message));

	memcpy(msg, &msg_struct, sizeof(message));

	if(write(clipboard_id, &msg, count)<0){
		exit(-1);
	}

	if(read(clipboard_id+1, &buf, sizeof(message))<0){
		exit(-1);
	}

	free(msg);
	return 0;
}
