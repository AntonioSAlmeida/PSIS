# PSIS
António Almeida N°78494
João Alves N°78181

Systems Programming Course Project

Shared clipboard 

Compile: 
gcc -c library.c -o library.o
gcc -pthread clipboard.c library.o -o clipboard
gcc app_teste.c library.o -o app_teste
