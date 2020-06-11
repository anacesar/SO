CC = gcc
CFLAGS = -Wall -g

all : fifos files server client


server: server.o utilities.o

server.o: server.c

client: client.o utilities.o

client.o: client.c

printer: printer.o utilities.o

printer.o: printer.c

utilities.o: utilities.c

fifos: 
	rm -f tmp/*

files:
	mkdir -p tmp
	mkdir -p Files
	touch Files/log.idx
	touch Files/log
	touch Files/errors
	chmod 0700 Files/*

clean: 
	rm -f *.o server client tmp/* 

delete:
	rm -f Files/*
	rm -f fiftmp/*
	rm -d Files
	rm -d tmp

