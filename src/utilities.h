#ifndef __UTILITIES__
#define __UTILITIES__

#include <unistd.h> /* chamadas ao sistema: defs e decls essenciais */
#include <fcntl.h> /* O_RDONLY, O_WRONLY, O_CREAT, O_* */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <limits.h>
#include <errno.h>
#include <time.h>


#define SIZE 1024
#define PATH_SIZE 64
#define OUT 256
#define CACHE 4096

#define FIFOS "tmp/fifo"
#define LOGS "files/logs" //file saving tasks output 
#define LOGIDX "files/log.idx" //file keeping structs task

typedef struct task {
	int id;
    int state; //0->finished, 1->user interrupcion, 2->max inactivity, 3->max execution, 4->executing
	size_t beginning; //where output beggins in log file
	size_t size; //size of output 
	char command[SIZE];
}Task; 

typedef struct circularArray{
	Task *tasks;
	int size;
	int currentPos;
} CircularArray;


ssize_t readln(int fd, char *line, size_t size);

void sendMyPid(int fifo);

pid_t getPidFromPath(char* path);

char** words(const char* line, int *size);

int countTaks(size_t* currentpos);

CircularArray* initCA(int size);

void insertCA(CircularArray *c, int id, char* command);

#endif