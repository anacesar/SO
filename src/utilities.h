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


#define BUFFER_SIZE 1024
#define PATH_SIZE 64
#define COMMAND_SIZE 1024
#define OUT 256

#define FIFOS "tmp/fifo"

typedef struct task *Task; 


ssize_t readln(int fd, char *line, size_t size);

void sendMyPid(int fifo);

pid_t getPidFromPath(char* path);

char** words(const char* line, int *size);

#endif