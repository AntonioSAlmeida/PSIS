/* Header Inclusions                                              */
#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>




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

 
void insertLinkedList(LinkedList ** head, pthread_t id, int fd){
  LinkedList * aux = NULL;
  LinkedList * aux2 = NULL;

  if(*head==NULL){
    aux = (LinkedList*)malloc(sizeof(LinkedList));
    aux->id=id;
    aux->fd=fd;
    aux->next=NULL;
    *head=aux;
  }else{
    aux=*head;
    aux2=(LinkedList*)malloc(sizeof(LinkedList));
    aux2->id=id;
    aux2->fd=fd;
    aux2->next=NULL;
    
    while(aux->next!=NULL){
      aux=aux->next;
    }

    aux->next=aux2;
  }
  
}




void printLinkedList(LinkedList * first)
{
  LinkedList * aux = first;
  while(aux!=NULL){
    //printf("thread: %lu\n", aux->id);
    printf("\nfd: %d\n", aux->fd);
    aux=aux->next;  
  }
}

void removeFromlist(LinkedList ** first, int fd)
{
  LinkedList * aux = *first;
  LinkedList * after = NULL;

  if(aux->fd==fd){
    *first=aux->next;
    free(aux);
  }else{
    while(aux->next->fd!=fd){
      aux=aux->next;  
    }
    after=aux->next->next;
    free(aux->next);
    aux->next=after;
  }
}

void freeList(LinkedList ** first){
  LinkedList * aux= *first;
  LinkedList * aux2 = NULL;
  while(aux!=NULL){
    close(aux->fd);
    aux2=aux->next;
    free(aux);
    aux=aux2;
  }
}