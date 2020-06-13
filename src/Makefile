CC = gcc
CFLAGS = -Wall -g

all : fifos server client

server: server.o utilities.o

server.o: server.c

client: client.o utilities.o

client.o: client.c

utilities.o: utilities.c

fifos: 
	mkdir -p tmp
	mkdir -p files

clean: 
	rm -f *.o server client tmp/* 





