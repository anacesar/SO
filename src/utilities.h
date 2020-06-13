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
#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include <time.h>


#define SIZE 1024
#define PATH_SIZE 64
#define OUT 256
#define CACHE 4096
#define PUT_LINE    write(1, "\n", 1);

#define MAXCOMMANDS 10
#define MAXARGS 20

#define FIFOS "tmp/fifo"
#define TMP "tmp/output_"
#define LOG "files/log" 
#define LOGIDX "files/log.idx" 

typedef struct task {
	int id;
    int state; //0->finished, 1->executing, 2->user interrupcion, 3->max inactivity, 4->max execution, 5->error executing 
	int beginning; //where output beggins in log file
	int size; //size of output 
	char command[SIZE];
}Task; 

typedef struct circularArray{
	Task** tasks;
	int size;
	int currentPos;
}CircularArray;


ssize_t readln(int fd, char *line, size_t size);

void sendMyPid(int fifo);

pid_t getPidFromPath(char* path);

char* outputFile(int pid);

char** words(const char* line, int *size);

int countTaks(int* currentpos);

int finishTask(Task* task, int status, int init, int size);

CircularArray* initCA(int size);

Task* insertCA(CircularArray* c, int id, char* command);

int exec_command(char* command);

int getcommands(char* line, char** commands);

#endif