#include <sys/types.h>
#include "LinkedList.h"
#include <pthread.h>

#define INBOUND_FIFO "INBOUND_FIFO"
#define OUTBOUND_FIFO "OUTBOUND_FIFO"
#define CLIPBOARD_SOCKET "/tmp/unixsocket"
#define GATEWAY_PORT 8010


#define COPY 0
#define PASTE 1
#define UPDATE 2
#define SHUTDOWN 3
#define MAXSIZE 1024


typedef struct Message{
	int order;
	int region;
	int data_size;
} message;


int clipboard_connect(struct sockaddr_un socket_addr);
int clipboard_copy(int clipboard_id, int region, void *buf, size_t count);
int clipboard_paste(int clipboard_id, int region, void *buf, size_t count);
int clipboard_wait(int clipboard_id, int region, void *buf, size_t count);
void clipboard_close(int clipboard_id);