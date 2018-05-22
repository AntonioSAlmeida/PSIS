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
  struct LinkedListStruct * next;
} LinkedList;


int lengthLinkedList(LinkedList * head);

void insertFirstLinkedList(LinkedList ** head, pthread_t id);

void insertLinkedList(LinkedList ** head, pthread_t id);

/*Print List*/
void printLinkedList(LinkedList * first);

/* End of: Protect multiple inclusions                              */
#endif
