#include <sys/types.h>
#include "LinkedList.h"
#include <pthread.h>

#define INBOUND_FIFO "INBOUND_FIFO"
#define OUTBOUND_FIFO "OUTBOUND_FIFO"
#define CLIPBOARD_SOCKET "./sock_16"
#define REMOTE_PORT 1337


#define COPY 0
#define PASTE 1


typedef struct Message{
	int order;
	int region;
	int data_size;
} message;

typedef struct Connection{
	pthread_t id;
} connection;

int clipboard_connect(struct sockaddr_un socket_addr);
int clipboard_copy(int clipboard_id, int region, void *buf, size_t count);
int clipboard_paste(int clipboard_id, int region, void *buf, size_t count);
int clipboard_wait(int clipboard_id, int region, void *buf, size_t count);
void clipboard_close(int clipboard_id);