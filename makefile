all:
	gcc -c library.c -o library.o
	gcc -pthread clipboard.c library.o -o clipboard
	gcc app_teste.c library.o -o app_teste

clean:
	rm *.o app_teste clipboard sock_16