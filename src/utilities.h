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
 
/*server defines*/
#define BUFSIZE 200
#define MAXCOMMANDS 10
#define MAXARGS 20
#define MAX_READER 1024

/*utilities defines*/
#define COMMAND_SIZE   1024
#define PUT_LINE    write(1, "\n", 1);
#define CACHE       4096

#define FIFO "./tmp/fifo"
#define CLIENT_FIFO "./tmp/fifo"
#define S2CFIFO "./tmp/serv2cli"
#define ERRORS  "./Files/errors"

struct task {
	int id;
	int beginnig; //where output beggins in log file
	int size; //size of output 
	char command[COMMAND_SIZE];
}*Task;

typedef struct hashTable{
	struct Task *tasks;
} HashTable;

typedef struct circularArray{
	struct Task *tasks;
	int size;
	int currentPos;
} CircularArray;

char** words(const char* string, int *size);

int mypopen(char* command, char* type);

//ssize_t readln(int fildes, void *buf, size_t bytes);

ssize_t readln(int fd, char *line, size_t size);

/*
CircularArray* initCA(int size);

void insertCA(CircularArray *c, int artCode, float price);

void insertCAintoAG(CircularArray *c, int artCode, float totalSold, int artQuantity);

float findCA(CircularArray *c, int artCode);

void freeCircularArray(CircularArray *c);

void flushCircularArray(CircularArray *c, int control);

char* timeToString(void);

int getGlobals(long long *code, long long *next);

void readVenda(struct venda v);

struct artigo readArtigo(long long artCode);

void display(struct artigo art);

pid_t getPidFromPath(char* path);

void getServerPath(char* serverPath, int serverPid);
*/

#endif