all:
	gcc -c library.c -o library.o
	gcc -c LinkedList.c -o LinkedList.o
	gcc -pthread clipboard.c library.o LinkedList.o -o clipboard
	gcc app_teste.c library.o -o app_teste
	gcc app_teste2.c library.o -o app_teste2
	gcc app_teste3.c library.o -o app_teste3
	gcc app_teste3.c library.o -o app_teste4
clean:
	rm *.o app_teste app_teste2 app_teste3 clipboard unixsocket
