#ifndef clipboardHeader
#define clipboardHeader


#include <sys/types.h>
#include "LinkedList.h"
#include <pthread.h>


#define INBOUND_FIFO "INBOUND_FIFO"
#define OUTBOUND_FIFO "OUTBOUND_FIFO"
#define CLIPBOARD_SOCKET "unixsocket"
#define GATEWAY_PORT 8010

#define COPY 0
#define PASTE 1
#define WAIT 2
#define UPDATE 3
#define SHUTDOWN 4



int clipboard_connect(struct sockaddr_un socket_addr);
int clipboard_copy(int clipboard_id, int region, void *buf, size_t count);
int clipboard_paste(int clipboard_id, int region, void *buf, size_t count);
int clipboard_wait(int clipboard_id, int region, void *buf, size_t count);
void clipboard_close(int clipboard_id);

#endif
