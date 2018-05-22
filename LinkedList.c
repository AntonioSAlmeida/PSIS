/* Header Inclusions                                              */
#include<stdio.h>
#include<stdlib.h>



#include"LinkedList.h"

 
int lengthLinkedList(LinkedList * head)
{
  LinkedList * aux;
  int counter=0;

  aux=head;
  while(aux!=NULL){
    counter++;
    aux=aux->next;
  }

  return counter;
}


void insertFirstLinkedList(LinkedList ** head, pthread_t id){
  LinkedList * aux = (LinkedList*)malloc(sizeof(LinkedList));
  aux->id=id;
  aux->next=NULL;
  *head=aux;
}
 
void insertLinkedList(LinkedList ** head, pthread_t id){
  LinkedList * aux = NULL;
  aux=*head;
  
  while(aux->next!=NULL){
    aux=aux->next;
  }

  aux->next=(LinkedList*)malloc(sizeof(LinkedList));
  aux->next->id=id;
  aux->next->next=NULL;
}




void printLinkedList(LinkedList * first)
{
  LinkedList * aux = first;
  while(aux!=NULL){
    printf("%lu\n", aux->id);
    aux=aux->next;  
  }
}
