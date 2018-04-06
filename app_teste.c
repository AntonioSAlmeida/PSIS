#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "clipboard.h"

int main(){

	int fd = clipboard_connect("./");

	if(fd== -1){
		exit(-1);
	}

	char dados[10];
	int dados_int;
	fgets(dados, 10, stdin);

	if(clipboard_copy(fd, 0, dados, sizeof(dados)) <0){
		exit(-1);
	}

	printf("still raidin\n");

	if(read(fd+1, &dados_int, sizeof(dados_int))<0){
			exit(-1);
	}
	printf("Received %d\n", dados_int);

	exit(0);
}
