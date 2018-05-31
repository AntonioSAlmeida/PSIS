/* Prevent multiple inclusions                                      */
#ifndef LinkedListHeader
#define LinkedListHeader






/*
 *  Data type: LinkedList
 *
 *  Description: Node of a linked list
 */


typedef struct LinkedListStruct
{
  pthread_t id;
  int fd;
  struct LinkedListStruct * next;
} LinkedList;


int lengthLinkedList(LinkedList * head);

void insertLinkedList(LinkedList ** head, pthread_t id, int fd);

/*Print List*/
void printLinkedList(LinkedList * first);

void removeFromlist(LinkedList ** first, int fd);

void freeList(LinkedList ** first);

/* End of: Protect multiple inclusions                              */
#endif
