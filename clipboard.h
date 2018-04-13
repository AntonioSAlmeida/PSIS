/*This is actually library.h*/

#define INBOUND_FIFO "INBOUND_FIFO"
#define OUTBOUND_FIFO "OUTBOUND_FIFO"
#include <sys/types.h>


#define SOCK_ADDRESS "./sock_16"

typedef struct Message{
	int order;
	int region;
	int data_size;

} message;
int clipboard_connect(struct sockaddr_un socket_addr);
int clipboard_copy(int clipboard_id, int region, void *buf, size_t count);
int clipboard_paste(int clipboard_id, int region, void *buf, size_t count);
