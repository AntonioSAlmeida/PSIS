#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/wait.h>
#include <signal.h>

#include "LinkedList.h"
#include "library.h"

char **clipboard_content;
pthread_t id_thread;
LinkedList * localhead = NULL;
LinkedList * remotehead = NULL;

pthread_rwlock_t lock;


pthread_rwlock_t lock=PTHREAD_RWLOCK_INITIALIZER;


int clipboard_recv(int fd){

	int i = 0;
	int count = 0;
	int nbytes = 0;

	message	warning;
	char* warning_msg = (char*)malloc(sizeof(message));
	if(warning_msg==NULL)                     
    {
        printf("Error! memory not allocated.");
        exit(1);
    }

	pthread_rwlock_rdlock(&lock);//blocks read and write

	for(i = 0; i < 10; i++){

		nbytes = recv(fd, warning_msg, sizeof(message), 0);
		if(nbytes==-1){
			perror("Recv: ");
			exit(1);
		}

		memcpy(&warning, warning_msg, sizeof(message));

		count = warning.data_size;
		if(count>0){
			char* buffdata = malloc(warning.data_size);
			if(buffdata==NULL)                     
    		{
        		printf("Error! memory not allocated.");
        		exit(1);
    		}

			nbytes = recv(fd, buffdata, count, 0);
			if(nbytes==-1){
				perror("Recv: ");
				exit(1);
			}
			//Write in dynamic array
			if(warning.data_size>0){
			
				clipboard_content[i] = malloc(warning.data_size);
				if(buffdata==NULL){
        			printf("Error! memory not allocated.");
        			exit(1);
    			}
				memcpy(clipboard_content[i], buffdata, warning.data_size);
				
			}


			free(buffdata);
		}

	}
	pthread_rwlock_unlock(&lock);//unlocks

	free(warning_msg);

	return 1;
}


int clipboard_send(int fd){

	int i = 0;
	int nbytes=0;


	message	warning;

	char *warning_msg = (char*)malloc(sizeof(message));
	if(warning_msg==NULL){
       	printf("Error! memory not allocated.");
     	exit(1);
    }

	pthread_rwlock_wrlock(&lock);//blocks write
	for(i = 0; i < 10; i++){
		warning.order = UPDATE;
		warning.region = i;
		warning.data_size=0;
		if(clipboard_content[i]!=NULL)
			warning.data_size = (strlen(clipboard_content[i])+1)*sizeof(char);
		
		memcpy(warning_msg, &warning, sizeof(warning));
		
		send(fd, warning_msg, sizeof(message), 0);
		if(nbytes==-1){
			perror("Send: ");;
			exit(1);
		}

		if(warning.data_size!=0){
			send(fd, clipboard_content[i], warning.data_size, 0);
			if(nbytes==-1){
				perror("Send: ");
     			exit(1);
			}
		}
	}
	pthread_rwlock_unlock(&lock);//unblocks

	free(warning_msg);
	return 0;

}



void clipboard_shutdown(LinkedList * remotehead){
	message	warning;
	int nbytes=0;
	LinkedList * aux = remotehead;

	char *warning_msg = (char*)malloc(sizeof(message));
	if(warning_msg==NULL){
       	printf("Error! memory not allocated.");
     	exit(1);
    }

	while(aux!=NULL){
		warning.order = SHUTDOWN;
		warning.region = 0;
		warning.data_size=0;
	
		memcpy(warning_msg, &warning, sizeof(warning));
		
		send(aux->fd, warning_msg, sizeof(message), 0);
		if(nbytes==-1){
			perror("Send: ");
     		exit(1);
		}
		printf("sent shutdown\n");
		aux=aux->next;
	}
	
	
	free(warning_msg);
	return;

}

int update_broadcast(LinkedList * remote_connections, int region, int fd_received){

	int i = 0;

	message	warning;
	warning.order = UPDATE;
	warning.region = region;

	char *warning_msg = (char*)malloc(sizeof(message));
	if(warning_msg==NULL){
       	printf("Error! memory not allocated.");
     	exit(1);
    }
	LinkedList * aux = remote_connections;


	pthread_rwlock_wrlock(&lock);//blocks write

	warning.data_size = (strlen(clipboard_content[region])+1)*sizeof(char);
	
	memcpy(warning_msg, &warning, sizeof(message));


	while(aux != NULL){
		if(aux->fd!=fd_received){
			if(send(aux->fd, warning_msg, sizeof(warning), 0) < 0){
				perror("send: ");
				// free(region_copy);
				free(warning_msg);
				exit(1);
			}

			if(send(aux->fd, clipboard_content[region], warning.data_size, 0)<0){
				perror("send: ");
				// free(region_copy);
				free(warning_msg);
				exit(1);
			}
		}
		aux=aux->next;
	}
	pthread_rwlock_unlock(&lock);//unblocks write

	// free(region_copy);
	free(warning_msg);
	return 1;
}



void * local_thread_code(void * fdi){

		int * fd = fdi;
		int client_fd=*fd;
		char * buffstruct=malloc(sizeof(message));
		if(buffstruct==NULL){
       		printf("Error! memory not allocated.");
     		exit(1);
    	}

		int nbytes=1;
		int nbytes2=0;
		int count=0;
		message *message_size=malloc(sizeof(message));
		if(message_size==NULL){
       		printf("Error! memory not allocated.");
     		exit(1);
    	}

		// Receive messages - 1st) size of message; 2nd) message to coppdate_broadcast(remotehead, message_size->y to clipboard
		while(nbytes!=-1){
			
			nbytes = recv(client_fd, buffstruct, sizeof(message), 0);

			if(nbytes==0){
				break;
			}
			if(nbytes!=-1){


				//Build struct
				memcpy(message_size, buffstruct, sizeof(message));

				if(message_size->order==COPY){

					count=message_size->data_size;

					char * buffdata=malloc(sizeof(message_size->data_size));
					if(buffdata==NULL){
       					printf("Error! memory not allocated.");
     					exit(1);
    				}

					while(count>0){
						nbytes = recv(client_fd, buffdata, message_size->data_size, 0);
						count-=nbytes;
					}

					printf("\nCopy\nsize:%d\nregion:%d\nmessage:%s\n", message_size->data_size, message_size->region, buffdata);
					

					int bytes=0; //just so that realloc doesn't give a warning
					//Write in dynamic array
					pthread_rwlock_rdlock(&lock);//blocks write and read
				
					clipboard_content[message_size->region]=realloc(clipboard_content[message_size->region], message_size->data_size*sizeof(char));
					if(clipboard_content[message_size->region]==NULL){
						printf("Error! memory not allocated.");
						exit(1);
					}
					
					memcpy(clipboard_content[message_size->region], buffdata, message_size->data_size);
					pthread_rwlock_unlock(&lock);//unblocks write and read

					free(buffdata);

					if(remotehead!=NULL){
						printf("Broadcasting changes\n");
						update_broadcast(remotehead, message_size->region, -1);
					}


				}else if(message_size->order==PASTE){

					printf("\nPaste\nmessage:%s\n",clipboard_content[message_size->region]);

					//Sending message length

					memcpy(buffstruct, message_size, sizeof(message));

					nbytes2=send(client_fd, buffstruct, sizeof(message), 0);
					if(nbytes2==-1){
						perror("Send: ");
     					exit(1);
					}


					//Sending message

					char * buffdata=malloc(sizeof(message_size->data_size));
					if(buffdata==NULL){
						printf("Error! memory not allocated.");
     					exit(1);
					}

					pthread_rwlock_wrlock(&lock);//blocks write

					memcpy(buffdata, clipboard_content[message_size->region], message_size->data_size);

					pthread_rwlock_unlock(&lock);//unblocks write

					nbytes2=send(client_fd, buffdata, message_size->data_size, 0);
					if(nbytes2==-1){
						perror("Send: ");
     					exit(1);
					}

					free(buffdata);


				}

			}
		}
		free(buffstruct);
		free(message_size);
		close(client_fd);
		removeFromlist(&localhead, client_fd);
}



void * connected_thread_code(void * fdi){

	int j = 0;
	int * fd = fdi;
	int client_fd=*fd;
	char * buffstruct=malloc(100*sizeof(char));
	if(buffstruct==NULL){
		printf("Error! memory not allocated.");
		exit(1);
	}

	int nbytes=1;
	int count=0;
	message *message_size=malloc(sizeof(message));
	if(message_size==NULL){
		printf("Error! memory not allocated.");
		exit(1);
	}

	printf("connected thread live\n");
	// Receive messages - 1st) size of message; 2nd) message to copy to clipboard

	while(nbytes!=-1){
		
		nbytes = recv(client_fd, buffstruct, sizeof(message), 0);
		
		
		//Build struct
		memcpy(message_size, buffstruct, sizeof(message));

		if(message_size->order == UPDATE){

			count = message_size->data_size;

			char* buffdata = malloc(message_size->data_size);
			if(buffdata==NULL){
				printf("Error! memory not allocated.");
				exit(1);
			}

			nbytes = recv(client_fd, buffdata, message_size->data_size, 0);

			printf("\nUpdate\nsize:%d\nregion:%d\nmessage:%s\n", message_size->data_size, message_size->region, buffdata);

			pthread_rwlock_rdlock(&lock);//blocks write and read
			//Write in dynamic array
			clipboard_content[message_size->region] = realloc(clipboard_content[message_size->region], message_size->data_size);
			if(clipboard_content[message_size->region]==NULL){
				printf("Error! memory not allocated.");
				exit(1);
			}
			memcpy(clipboard_content[message_size->region], buffdata, message_size->data_size);

			pthread_rwlock_unlock(&lock);//unblocks write and reads

			free(buffdata);

			pthread_rwlock_wrlock(&lock);//blocks write
			for( j = 0; j < 10; j++){
				if(clipboard_content[j]!=NULL)
					printf("updated [%d] message: %s\n", j, clipboard_content[j]);
			}
			pthread_rwlock_unlock(&lock);//unblocks write

			update_broadcast(remotehead, message_size->region, client_fd);

		 }else if(message_size->order == SHUTDOWN){
		 	printf("\nfd: %d has shutdown\n", client_fd);
		 	break;
		 }
		 

	}
	free(buffstruct);
	free(message_size);
	removeFromlist(&remotehead, client_fd);
	close(client_fd);
}






void * remote_thread_code(void * fdi){

	int j = 0;
	int * fd = fdi;
	int client_fd=*fd;
	char * buffstruct=malloc(100*sizeof(char));
	if(buffstruct==NULL){
		printf("Error! memory not allocated.");
		exit(1);
	}

	int nbytes=1;
	int count=0;
	message *message_size=malloc(sizeof(message));
	if(message_size==NULL){
		printf("Error! memory not allocated.");
		exit(1);
	}

	printf("remote thread live \n");
	// Receive messages - 1st) size of message; 2nd) message to copy to clipboard

	
	clipboard_send(client_fd);

	while(nbytes!=-1){
		
		nbytes = recv(client_fd, buffstruct, sizeof(message), 0);
		
		//Build struct
		memcpy(message_size, buffstruct, sizeof(message));

		if(message_size->order == UPDATE){

			count = message_size->data_size;

			char* buffdata = malloc(message_size->data_size);

			nbytes = recv(client_fd, buffdata, message_size->data_size, 0);

			printf("\nUpdate\nsize:%d\nregion:%d\nmessage:%s\n", message_size->data_size, message_size->region, buffdata);

			pthread_rwlock_rdlock(&lock);//blocks write and read
			//Write in dynamic array
			clipboard_content[message_size->region] = realloc(clipboard_content[message_size->region] , message_size->data_size);
			if(clipboard_content[message_size->region]==NULL){
				printf("Error! memory not allocated.");
				exit(1);
			}


			memcpy(clipboard_content[message_size->region], buffdata, message_size->data_size);
	

			free(buffdata);


			for( j = 0; j < 10; j++){
				if(clipboard_content[j]!=NULL)
					printf("updated [%d] message: %s\n", j, clipboard_content[j]);
			}
			pthread_rwlock_unlock(&lock);//unblocks write and read

			update_broadcast(remotehead, message_size->region, client_fd);

		 }else if(message_size->order == SHUTDOWN){
		 	printf("fd: %d shutdown\n", client_fd);
		 	break;
		 }
		 

	}
	free(buffstruct);
	free(message_size);
	removeFromlist(&remotehead, client_fd);
	close(client_fd);
}






void * accept_local_connection(){



	struct sockaddr_un server_addr;
	struct sockaddr_un client_addr;
	socklen_t size_addr;

	// Create unix domain socket-stream socket
	int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock_fd == -1){
		perror("unix socket: ");
		exit(-1);
	}

	// Fill server_addr
	server_addr.sun_family = AF_UNIX;
	strcpy(server_addr.sun_path, CLIPBOARD_SOCKET);

	//Unlink & Bind socket to address server_addr
	unlink(CLIPBOARD_SOCKET);
	int err = bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if(err == -1) {
		perror("bind unix ");
		exit(-1);
	}

	// Listen
	listen(sock_fd, 5);
	printf("Ready to accept local connections\n");

	int* c_fd = malloc(sizeof(int));
	if(c_fd==NULL){
		printf("Error! memory not allocated.");
		exit(1);
	}

	int client_fd=-2;


	while(client_fd!=-1){

		// Accept
		client_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &size_addr);

		printf("Accepted one connection from %s\n", CLIPBOARD_SOCKET);
		//Call new thread
		*c_fd=client_fd;

		pthread_create(&id_thread, NULL, local_thread_code, c_fd);

		pthread_rwlock_rdlock(&lock);//blocks write and read
  		insertLinkedList(&localhead, id_thread, client_fd);

		printLinkedList(localhead);
		pthread_rwlock_unlock(&lock);//unblocks write

	}

	close(sock_fd);
	free(c_fd);


}


void * accept_remote_connection(){

	//Create internet socket

	struct sockaddr_in gateway_local_addr;
	struct sockaddr_in conn_local_addr;
  	struct sockaddr_in client_addr;
  	socklen_t size_addr;

  	int nbytes;

  	int gateway_inet_sock_fd= socket(AF_INET, SOCK_STREAM, 0);
  	if (setsockopt(gateway_inet_sock_fd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0){
    	printf("setsockopt(SO_REUSEADDR) failed");
  	}

  	if (gateway_inet_sock_fd == -1){
    	perror("socket: ");
    	exit(-1);
  	}
  	char * buffer=malloc(1024*sizeof(char));
  	if(buffer==NULL){
		printf("Error! memory not allocated.");
		exit(1);
	}

  	//Fill socket that is the gateway
  	gateway_local_addr.sin_family = AF_INET;
  	gateway_local_addr.sin_port= htons(GATEWAY_PORT);
  	gateway_local_addr.sin_addr.s_addr= INADDR_ANY;

  	//Gateway port is defined by default
  	int gateway_port=GATEWAY_PORT;


  	//Change the gateway port if the default one is taken

  	while(bind(gateway_inet_sock_fd, (struct sockaddr *)&gateway_local_addr, sizeof(gateway_local_addr))==-1){
  		gateway_port++;
  		gateway_local_addr.sin_port= htons(gateway_port);
  	};

  	printf("my gateway port is %d\n", gateway_port);

  	//Listen
  	listen(gateway_inet_sock_fd, 5);

  	printf("Ready to accept remote clipboard connections\n");

  	int* c_fd = malloc(sizeof(int));
  	if(c_fd==NULL){
		printf("Error! memory not allocated.");
		exit(1);
	}
	int gateway_client_fd=-2;
	int client_fd;
	int inet_sock_fd;
	char * port;
	int local_tcp_port;
	nbytes=1;
	port = malloc(10*sizeof(char));
	if(port==NULL){
		printf("Error! memory not allocated.");
		exit(1);
	}

  	while(gateway_client_fd!=-1){

    	gateway_client_fd=accept(gateway_inet_sock_fd, (struct sockaddr *) & client_addr, &size_addr);

    	printf("Accepted one connection from remote clipboard\n");

    	inet_sock_fd= socket(AF_INET, SOCK_STREAM, 0);
  		if (setsockopt(inet_sock_fd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0){
    		printf("setsockopt(SO_REUSEADDR) failed");
  		}

  		if (inet_sock_fd == -1){
    		perror("socket: ");
    		exit(-1);
  		}

  		
  		local_tcp_port=gateway_port;

  		conn_local_addr.sin_family = AF_INET;
  		conn_local_addr.sin_port= htons(local_tcp_port);
  		conn_local_addr.sin_addr.s_addr= INADDR_ANY;

  		while(bind(inet_sock_fd, (struct sockaddr *)&conn_local_addr, sizeof(conn_local_addr))==-1){
  			local_tcp_port++;
  			conn_local_addr.sin_port= htons(local_tcp_port);
  		};



  		listen(inet_sock_fd, 5);
  		//Sending port to new connection
  		sprintf(port, "%d", local_tcp_port);

    	nbytes = send(gateway_client_fd, port, 10*sizeof(char), 0);
		if(nbytes==0){
			break;
		}

		nbytes=1;

		nbytes=recv(gateway_client_fd, buffer, sizeof(char)*20, 0);

		if(strcmp(buffer, "received")!=0){
			printf("port was not received by remote clipboard\n");
			exit(1);
		}

		client_fd= accept(inet_sock_fd, (struct sockaddr *) &client_addr, &size_addr);
		if(client_fd==-1){
			perror("accept ");
			exit(-1);
		}
    	//Call new thread
		*c_fd=client_fd;

		pthread_create(&id_thread, NULL, remote_thread_code, c_fd);

		pthread_rwlock_rdlock(&lock);//blocks write and read
  		insertLinkedList(&remotehead, id_thread, client_fd);
 
		printLinkedList(remotehead);
		pthread_rwlock_unlock(&lock);//unblocks write

	}

	close(gateway_inet_sock_fd);
	free(buffer);
	free(c_fd);
	close(gateway_client_fd);
	close(inet_sock_fd);
	free(port);


}



int main(int argc, char *argv[]){




	int remote_fd = -1;
	int gateway_remote_fd=-1;
	int j=0;

	pthread_rwlock_rdlock(&lock);//blocks write and read
	clipboard_content = malloc(10*sizeof(char*));
	if(clipboard_content==NULL){
		printf("Error! memory not allocated.");
		exit(1);
	}

	for(j=0; j<10; j++){
		clipboard_content[j]=NULL;
	}
	pthread_rwlock_unlock(&lock);//unblocks write and read


	


	if(argc > 1){
		if(!strcmp(argv[1], "-c")){
			printf("in connected mode\n");
			struct sockaddr_in remote_addr;

			// Create connection to socket to communicate with remote clipboard
			gateway_remote_fd = socket(AF_INET, SOCK_STREAM, 0);

			if (setsockopt(gateway_remote_fd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0){
		    	printf("setsockopt(SO_REUSEADDR) failed");
  			}


			if (gateway_remote_fd == -1){
				perror("socket: ");
				exit(-1);
			}

			remote_addr.sin_family = AF_INET;
			remote_addr.sin_port = htons(atoi(argv[3]));
			inet_aton(argv[2], &remote_addr.sin_addr);

			if(-1 == connect(gateway_remote_fd, (struct sockaddr *) &remote_addr, sizeof(remote_addr))){
				perror("connect: ");
				exit(-1);
			}

			char * port = malloc(10*sizeof(char));
			if(port==NULL){
				printf("Error! memory not allocated.");
				exit(1);
			}

			int nbytes=1;

			nbytes=recv(gateway_remote_fd, port, 10*sizeof(char), 0);

			if(nbytes==-1){
				perror("recv: ");
				exit(1);
			}

			int remote_port=atoi(port);

			char buffer[20];
			buffer[0]='\0';
			sprintf(buffer, "received");
			nbytes=1;

	    	nbytes = send(gateway_remote_fd, buffer, sizeof(char)*20, 0);
	    	if(nbytes==-1){
				perror("send: ");
				exit(1);
			}

			close(gateway_remote_fd);

			// Create connection to socket to communicate with remote clipboard
			remote_fd = socket(AF_INET, SOCK_STREAM, 0);

			if (setsockopt(remote_fd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0){
		    	printf("setsockopt(SO_REUSEADDR) failed");
  			}


			if (remote_fd == -1){
				perror("remote socket: ");
				exit(-1);
			}

			remote_addr.sin_family = AF_INET;
			remote_addr.sin_port = htons(remote_port);
			inet_aton(argv[2], &remote_addr.sin_addr);

			if(connect(remote_fd, (const struct sockaddr *) &remote_addr, sizeof(remote_addr))==-1){
				perror("connect remote: ");
				exit(-1);
			};



			printf("connected to remote clipboard\n");

			clipboard_recv(remote_fd);

			int * c_fd=malloc(sizeof(int));
			if(c_fd==NULL){
				printf("Error! memory not allocated.");
				exit(1);
			}

			*c_fd=remote_fd;

			pthread_create(&id_thread, NULL, connected_thread_code, c_fd);

			pthread_rwlock_rdlock(&lock);//blocks write and read
  			insertLinkedList(&remotehead, 0, remote_fd);

			printLinkedList(remotehead);
			pthread_rwlock_unlock(&lock);//unblocks write


			
			pthread_rwlock_wrlock(&lock);//blocks write
			for( j = 0; j < 10; j++){
				if(clipboard_content[j]!=NULL)
					printf("updated [%d] message: %s\n", j, clipboard_content[j]);
			}
			pthread_rwlock_unlock(&lock);//unblocks write

		}
	}

	pthread_create(&id_thread, NULL, accept_local_connection, NULL);

	pthread_create(&id_thread, NULL, accept_remote_connection, NULL);



	j=0;
	char * buffer=malloc(100*sizeof(char));
	while(1){
		fgets(buffer, 100*sizeof(char), stdin);
		if(strcmp(buffer, "exit\n")==0){
			clipboard_shutdown(remotehead);
			close(gateway_remote_fd);
			freeList(&remotehead);
			freeList(&localhead);

			pthread_rwlock_rdlock(&lock);//blocks write and read


			for(j=0; j<10; j++){
				free(clipboard_content[j]);
			}

			pthread_rwlock_unlock(&lock);//unblocks write and write


			exit(0);
		}else if(strcmp(buffer, "print clipboard\n")==0){
			pthread_rwlock_wrlock(&lock);//blocks write
			printf("\nClipboard:\n");
			for( j = 0; j < 10; j++){
				if(clipboard_content[j]!=NULL){
					
					printf("region [%d] message: %s\n", j, clipboard_content[j]);
				}
			}
			pthread_rwlock_unlock(&lock);//unblocks write
		}else if(strcmp(buffer, "print lists\n")==0){
			printf("\nApps connected:\n");
			pthread_rwlock_wrlock(&lock);//blocks write
			printLinkedList(localhead);
			printf("\nClipboards connected:\n");
			printLinkedList(remotehead);
			pthread_rwlock_unlock(&lock);//unblocks write
		}
	}

}
