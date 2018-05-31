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

// int endmain=0, endltc=0, endrtc=0, endalc=0, endarc=0;




// void terminate_main(){
// 	printf("f1\n");
// 	while(endmain!=2){}
// 	return;
// }

// void terminate_accept_remote_connection(){
// 	printf("f2\n");
// 	while(endarc!=lengthLinkedList(remotehead)){}
// 	return;
// }

// void terminate_accept_local_connection(){
// 	printf("f3\n");
// 	while(endarc!=lengthLinkedList(localhead)){}
// 	return;
// }

// void terminate_remote_thread_code(){
// 	printf("f4\n");
// 	endrtc=1;
// 	return;
// }

// void terminate_local_thread_code(){
// 	printf("f5\n");
// 	endltc=1;
// 	return;
// }


int sendregion(int fd, int region){
	send(fd, clipboard_content[region], sizeof(clipboard_content[region]), 0);
}

int clipboard_recv(int fd){

	int i = 0;
	int count = 0;
	int nbytes = 0;

	message	warning;
	char* warning_msg = (char*)malloc(sizeof(message));


	for(i = 0; i < 10; i++){

		nbytes = recv(fd, warning_msg, sizeof(message), 0);
		memcpy(&warning, warning_msg, sizeof(message));

		count = warning.data_size;
		if(count>0){
			char* buffdata = malloc(warning.data_size);
			nbytes = recv(fd, buffdata, count, 0);
			if(nbytes==-1){
				free(buffdata);
				break;
			}
			//Write in dynamic array
			if(warning.data_size>0){
				clipboard_content[i] = malloc(warning.data_size);
				memcpy(clipboard_content[i], buffdata, warning.data_size);
			}


			free(buffdata);
		}

	}
	free(warning_msg);

	return 1;
}


int clipboard_send(int fd){
	char **clip_copy = (char**)malloc(10*sizeof(char*));
	int i = 0;


	//start mutex here

	for(i = 0; i < 10; i++){

		if(clipboard_content[i] != NULL){
			clip_copy[i] = (char*)malloc(sizeof(clipboard_content[i]));
			memcpy(clip_copy[i], clipboard_content[i], (strlen(clipboard_content[i])+1)*sizeof(char));
		}
	}

	//free mutex here

	message	warning;


	char *warning_msg = (char*)malloc(sizeof(message));

	for(i = 0; i < 10; i++){
		warning.order = UPDATE;
		warning.region = i;
		warning.data_size=0;
		if(clipboard_content[i]!=NULL)
			warning.data_size = (strlen(clipboard_content[i])+1)*sizeof(char);
		memcpy(warning_msg, &warning, sizeof(warning));
		send(fd, warning_msg, sizeof(message), 0);
		if(warning.data_size!=0)
			send(fd, clip_copy[i], warning.data_size, 0);
	}

	for(i = 0; i < 10; i++){
		free(clip_copy[i]);
	}
	free(clip_copy);
	free(warning_msg);
	return 1;

}



void clipboard_shutdown(LinkedList * remotehead){
	message	warning;
	LinkedList * aux = remotehead;

	char *warning_msg = (char*)malloc(sizeof(message));

	while(aux!=NULL){
		warning.order = SHUTDOWN;
		warning.region = 0;
		warning.data_size=0;
	
		memcpy(warning_msg, &warning, sizeof(warning));
		
		send(aux->fd, warning_msg, sizeof(message), 0);
		printf("sent shutdown\n");
		aux=aux->next;
	}
	
	
	free(warning_msg);
	return;

}

int update_broadcast(LinkedList * remote_connections, int region, int fd_received){

	//start mutex
	char *region_copy = (char*)malloc((strlen(clipboard_content[region])+1)*sizeof(char));
	memcpy(region_copy, clipboard_content[region], (strlen(clipboard_content[region])+1)*sizeof(char));
	//end mutex

	int i = 0;

	message	warning;
	warning.order = UPDATE;
	warning.region = region;
	warning.data_size = (strlen(region_copy)+1)*sizeof(char);

	char *warning_msg = (char*)malloc(sizeof(message));

	memcpy(warning_msg, &warning, sizeof(message));


	LinkedList * aux = remote_connections;

	while(aux != NULL){
		if(aux->fd!=fd_received){
			if(send(aux->fd, warning_msg, sizeof(warning), 0) < 0){
				perror("send: ");
				free(region_copy);
				free(warning_msg);
				return -1;
			}

			if(send(aux->fd, region_copy, warning.data_size, 0)<0){
				perror("send: ");
				free(region_copy);
				free(warning_msg);
				return -1;
			}
		}
		aux=aux->next;
	}

	free(region_copy);
	free(warning_msg);
	return 1;
}



void * local_thread_code(void * fdi){

		int * fd = fdi;
		int client_fd=*fd;
		char * buffstruct=malloc(sizeof(message));

		int nbytes=1;
		int count=0;
		message *message_size=malloc(sizeof(message));

		// Receive messages - 1st) size of message; 2nd) message to coppdate_broadcast(remotehead, message_size->y to clipboard
		while(nbytes!=-1){
			// signal(SIGINT, terminate_local_thread_code);
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

					while(count>0){
						nbytes = recv(client_fd, buffdata, message_size->data_size, 0);
						count-=nbytes;
					}

					printf("\nCopy\nsize:%d\nregion:%d\nmessage:%s\n", message_size->data_size, message_size->region, buffdata);

					//Write in dynamic array
					clipboard_content[message_size->region] = malloc(message_size->data_size*sizeof(char));

					memcpy(clipboard_content[message_size->region], buffdata, message_size->data_size);

					free(buffdata);

					if(remotehead!=NULL){
						printf("Broadcasting changes\n");
						update_broadcast(remotehead, message_size->region, -1);
					}


				}else if(message_size->order==PASTE){

					printf("\nPaste\nmessage:%s\n",clipboard_content[message_size->region]);

					//Sending message length

					memcpy(buffstruct, message_size, sizeof(message));

					send(client_fd, buffstruct, sizeof(message), 0);


					//Sending message

					char * buffdata=malloc(sizeof(message_size->data_size));

					memcpy(buffdata, clipboard_content[message_size->region], message_size->data_size);

					send(client_fd, buffdata, message_size->data_size, 0);

					free(buffdata);


				}

				// if(endltc){
				// 	free(buffstruct);
				// 	free(message_size);
				// 	printf("Local connection closed on fd: %d. Removed from local connections list.\n", client_fd);
				// 	endalc++;
				// 	exit(0);

				// }
			}
		}
		free(buffstruct);
		free(message_size);
		close(client_fd);
		removeFromlist(&localhead, client_fd);
}



void * connected_thread_code(void * fdi){
	printf("connected thread live\n");
	int j = 0;
	int * fd = fdi;
	int client_fd=*fd;
	char * buffstruct=malloc(100*sizeof(char));

	int nbytes=1;
	int count=0;
	message *message_size=malloc(sizeof(message));

	printf("connected thread live\n");
	// Receive messages - 1st) size of message; 2nd) message to copy to clipboard

	while(1){
		// signal(SIGINT, terminate_local_thread_code);
		nbytes = recv(client_fd, buffstruct, sizeof(message), 0);
		
		
		if(nbytes!=-1){
			//Build struct
			memcpy(message_size, buffstruct, sizeof(message));

			if(message_size->order == UPDATE){

				count = message_size->data_size;

				char* buffdata = malloc(message_size->data_size);

				nbytes = recv(client_fd, buffdata, message_size->data_size, 0);

				printf("\nCopy\nsize:%d\nregion:%d\nmessage:%s\n", message_size->data_size, message_size->region, buffdata);

				//Write in dynamic array
				clipboard_content[message_size->region] = malloc(message_size->data_size);

				memcpy(clipboard_content[message_size->region], buffdata, message_size->data_size);

				free(buffdata);

				for( j = 0; j < 10; j++){
					printf("[%d] message: %s\n", j, clipboard_content[j]);
				}

				update_broadcast(remotehead, message_size->region, client_fd);

			 }else if(message_size->order == SHUTDOWN){
			 	printf("fd: %d shutdown\n", client_fd);
			 	break;
			 }
		 }
		// if(endrtc){
		// 	free(buffstruct);
		// 	printf("Remote connection closed on fd: %d. Removed from remote connections list.\n", client_fd);
		// 	endarc++;
		// 	exit(0);
		// }
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

	int nbytes=1;
	int count=0;
	message *message_size=malloc(sizeof(message));

	printf("remote thread live \n");
	// Receive messages - 1st) size of message; 2nd) message to copy to clipboard

	
	clipboard_send(client_fd);



	while(1){
		// signal(SIGINT, terminate_local_thread_code);
		nbytes = recv(client_fd, buffstruct, sizeof(message), 0);
		if(nbytes!=-1){
			//Build struct
			memcpy(message_size, buffstruct, sizeof(message));

			if(message_size->order == UPDATE){

				count = message_size->data_size;

				char* buffdata = malloc(message_size->data_size);

				nbytes = recv(client_fd, buffdata, message_size->data_size, 0);

				printf("\nCopy\nsize:%d\nregion:%d\nmessage:%s\n", message_size->data_size, message_size->region, buffdata);

				//Write in dynamic array
				clipboard_content[message_size->region] = malloc(message_size->data_size);

				memcpy(clipboard_content[message_size->region], buffdata, message_size->data_size);

				free(buffdata);

				for( j = 0; j < 10; j++){
					printf("[%d] message: %s\n", j, clipboard_content[j]);
				}

				update_broadcast(remotehead, message_size->region, client_fd);

			 }else if(message_size->order == SHUTDOWN){
			 	printf("fd: %d shutdown\n", client_fd);
			 	break;
			 }
		 }
		// if(endrtc){
		// 	free(buffstruct);
		// 	printf("Remote connection closed on fd: %d. Removed from remote connections list.\n", client_fd);
		// 	endarc++;
		// 	exit(0);
		// }
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
		perror("bind unix");
		exit(-1);
	}

	// Listen
	listen(sock_fd, 5);
	printf("Ready to accept local connections\n");

	int* c_fd = malloc(sizeof(int));
	int client_fd;


	while(1){
		// signal(SIGINT, terminate_accept_local_connection);

		// Accept
		client_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &size_addr);

		printf("Accepted one connection from %s\n", CLIPBOARD_SOCKET);
		//Call new thread
		*c_fd=client_fd;

		pthread_create(&id_thread, NULL, local_thread_code, c_fd);

  		insertLinkedList(&localhead, id_thread, client_fd);


		printf("local connections list size: %d\n", lengthLinkedList(localhead));
		printf("local connections list\n");
		printLinkedList(localhead);


		// if(endalc==lengthLinkedList(localhead)){
		// 	free(c_fd);
		// 	close(sock_fd);
		// 	endmain++;
		// 	exit(0);
		// }

	}


}


void * accept_remote_connection(){



	//Create internet socket

	struct sockaddr_in gateway_local_addr;
	struct sockaddr_in conn_local_addr;
  	struct sockaddr_in client_addr;
  	socklen_t size_addr;

  	char buff[100];
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
	int gateway_client_fd;
	int client_fd;
	nbytes=1;

  	while(1){
  		// signal(SIGINT, terminate_accept_remote_connection);
    	gateway_client_fd=accept(gateway_inet_sock_fd, (struct sockaddr *) & client_addr, &size_addr);

    	printf("Accepted one connection from remote clipboard\n");

    	int inet_sock_fd= socket(AF_INET, SOCK_STREAM, 0);
  		if (setsockopt(inet_sock_fd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0){
    		printf("setsockopt(SO_REUSEADDR) failed");
  		}

  		if (inet_sock_fd == -1){
    		perror("socket: ");
    		exit(-1);
  		}

  		char * buffer=malloc(1024*sizeof(char));

  		int local_tcp_port=GATEWAY_PORT;
  		conn_local_addr.sin_family = AF_INET;
  		conn_local_addr.sin_port= htons(local_tcp_port);
  		conn_local_addr.sin_addr.s_addr= INADDR_ANY;

  		while(bind(inet_sock_fd, (struct sockaddr *)&conn_local_addr, sizeof(conn_local_addr))==-1){
  			local_tcp_port++;
  			conn_local_addr.sin_port= htons(local_tcp_port);
  		};


  		close(inet_sock_fd);

  		inet_sock_fd= socket(AF_INET, SOCK_STREAM, 0);
  		if (setsockopt(inet_sock_fd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0){
    		printf("setsockopt(SO_REUSEADDR) failed");
  		}

  		if (inet_sock_fd == -1){
    		perror("socket: ");
    		exit(-1);
  		}

  		if(bind(inet_sock_fd, (struct sockaddr *)&conn_local_addr, sizeof(conn_local_addr))==-1){
  			perror("bind: ");
  			exit(-1);
  		}

  		listen(inet_sock_fd, 5);
  		//Sending port to new connection

  		char * port = malloc(10*sizeof(char));

  		sprintf(port, "%d", local_tcp_port);

    	nbytes = send(gateway_client_fd, port, 10*sizeof(char), 0);
		if(nbytes==0){
			break;
		}

		nbytes=1;

		nbytes=recv(gateway_client_fd, buffer, sizeof(char)*100, 0);
		if(nbytes=0){
			break;
		}

		if(strcmp(buffer, "port received")!=0){
			printf("port was not received by remote clipboard\n");
			exit(-1);
		}

		client_fd= accept(inet_sock_fd, (struct sockaddr *) &client_addr, &size_addr);
		if(client_fd==-1){
			perror("accept: ");
			exit(-1);
		}
    	//Call new thread
		*c_fd=client_fd;

		pthread_create(&id_thread, NULL, remote_thread_code, c_fd);

  		insertLinkedList(&remotehead, id_thread, client_fd);


		printf("remote connections list size: %d\n", lengthLinkedList(remotehead));
		printf("remote connections list:\n");
		printLinkedList(remotehead);


		// if(endarc==lengthLinkedList(remotehead)){
		// 	close(gateway_inet_sock_fd);
		// 	close(inet_sock_fd);
		// 	close(gateway_client_fd);
		// 	free(port);
		// 	free(buffer);
		// 	printf("no longer accepting local connections\n");
		// 	endmain++;
		// 	exit(0);
		// }
	}


}



int main(int argc, char *argv[]){




	int remote_fd = -1;
	int gateway_remote_fd=-1;
	


	clipboard_content = malloc(10*sizeof(char*));
	int j=0;

	for(j=0; j<10; j++){
		clipboard_content[j]=NULL;
	}


	pthread_create(&id_thread, NULL, accept_local_connection, NULL);

	pthread_create(&id_thread, NULL, accept_remote_connection, NULL);


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
				printf("Error connecting\n");
				exit(-1);
			}

			char * port = malloc(10*sizeof(char));

			int nbytes=1;

			nbytes=recv(gateway_remote_fd, port, 10*sizeof(char), 0);

			int remote_port=atoi(port);

			char buffer[100];
			sprintf(buffer, "port received");
			nbytes=1;

	    	nbytes = send(gateway_remote_fd, buffer, sizeof(char)*100, 0);

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
			*c_fd=remote_fd;

			pthread_create(&id_thread, NULL, connected_thread_code, c_fd);

  			insertLinkedList(&remotehead, 0, remote_fd);


			printf("remote connections list size: %d\n", lengthLinkedList(remotehead));
			printf("remote connections list:\n");
			printLinkedList(remotehead);


			

			for( j = 0; j < 10; j++){
				printf("[%d] message: %s\n", j, clipboard_content[j]);
			}

		}
	}



	j=0;
	char * buffer=malloc(100*sizeof(char));
	while(1){
		fgets(buffer, 100*sizeof(char), stdin);
		if(strcmp(buffer, "exit\n")==0){
			clipboard_shutdown(remotehead);
			close(gateway_remote_fd);
			freeList(&remotehead);
			freeList(&localhead);

			for(j=0; j<10; j++){
				free(clipboard_content[j]);
			}

			exit(0);
		}
	}

}
