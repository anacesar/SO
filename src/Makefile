CC = gcc
CFLAGS = -Wall -g

all : fifos servidor cliente

servidor: servidor.o utilities.o

servidor.o: servidor.c

cliente: cliente.o utilities.o

cliente.o: cliente.c

utilities.o: utilities.c

fifos: 
	mkdir -p tmp
	mkdir -p files

clean: 
	rm -f *.o servidor cliente tmp/* 



