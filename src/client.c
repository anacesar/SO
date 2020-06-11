#include "utilities.h"

//fifo -> fd for server2client fifo
//myfifo -> fd for fifo client creates to send read messages from server
int fifo, myfifo;
pid_t pid;
char keepPath[128];
char serverPath[128];

void closeCommunication(char* path){
	unlink(path);
	close(fifo);
}

void ctrl_c_handler(){
	closeCommunication(keepPath);
	close(myfifo);
	exit(0);
}

void ctrl_alarm_handler(){
	closeCommunication(keepPath);
	close(myfifo);
	exit(0);
}


int sendMyPid(){
    pid = getpid();
    char* pid_string = malloc(25);
    sprintf(pid_string, "%d", pid);
    if(write(fifo, pid_string, strlen(pid_string)) == -1) return -1;
    return 0;
}


/* prepare client to comunicate with server */
// 0--> sucess 
// -1--> error
int initCommunication(void){
	int fdel;
	/* Prepare signals */
	signal(SIGALRM, ctrl_alarm_handler);
	signal(SIGINT, ctrl_c_handler);
	//signal(SIGUSR1, sigusr1_handler);

	//getServerPath(serverPath, serverPid);

	char* path = malloc(sizeof(char) * 64);
	memset(path, '\0', 64);
	pid = getpid();
	snprintf(path, 64, "/tmp/fifo%ld", (long) pid);
	if((fdel = open(ERRORS, O_WRONLY)) == -1){
		write(2, "Rip error logging.\n", 19);
		return NULL;
	} else {
		dup2(fdel, 2);
		close(fdel);
	}
	if (mkfifo(path, 0777) == -1){
		write(2, "client fifo\n", 12);
		return NULL;
	}
	alarm(3);
	if((fifo = open(serverPath, O_WRONLY)) == -1){
		write(2, "server fifo\n", 13);
		return NULL;
	}
	alarm(0);

	strcpy(keepPath, path);

	return path;
}
int main(void){
	int n, bufferSize = 1024, offset = 0;
	char *path, *buffer = malloc(sizeof(char) * bufferSize);

	if((path = initCommunication()) == -1) return -1;

	/* Read cicle */
	while((n = readln(0, buffer + offset, bufferSize - offset)) > 0){
		offset += n;
		if ((offset && buffer[offset-1] == '\n') || buffer[n-1] == '\n') {
			buffer[offset-1] = '\0';
			if (buffer[0] == '\0' || parser(buffer, path)) break;
			offset = 0;
			memset(buffer, '\0', bufferSize);
		} else {
			buffer = realloc(buffer, bufferSize * 2);
            bufferSize *= 2;
		}
	}

	closeCommunication(path);

	free(path);
	free(buffer);

	return 0;
}