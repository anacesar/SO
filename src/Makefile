CC = gcc
CFLAGS = -Wall -g

all : fifos argus argusd

argusd: argusd.o utilities.o

argusd.o: argusd.c

argus: argus.o utilities.o

argus.o: argus.c

utilities.o: utilities.c

fifos: 
	mkdir -p tmp
	mkdir -p files
	touch files/log
	touch files/log.idx

clean: 
	rm -f *.o argus argusd tmp/* files/*
	rm -d tmp files