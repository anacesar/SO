#include "utilities.h"

/*for comunication*/
//char serverFifo[128];
pid_t clientPid;
int fifo, ser2cli, cli2serv;
char s2cPath[128];
char* c2sPath;

/*to deal with tasks*/
int ntaks = 0; 
int* pids;
int npids;

int exec_command(char* command){
    char* exec_args[MAXARGS];
    char* token; 
    int exec_ret = 0;
    int i = 0;

    token = strtok(command, " ");
    
    for(; token!=NULL ; token = strtok(NULL, " "), i++) exec_args[i] = token;

    exec_args[i] = NULL;

    exec_ret = execvp(exec_args[0], exec_args);
    return exec_ret;
}

int getcommands(char* line, char** commands){
    int i = 0;
    //doesnt give error without strdup, but why ??? c...
    commands[i++] = strdup(strtok(line, "|"));

    char* token; 

    //need to check for when nr commands > MAXCOMMANDS
    for(; (token = strtok(NULL, "|")) != NULL && i<MAXCOMMANDS ; i++) commands[i] = strdup(token);

    commands[i] = "\0";

    return i;
}

int initServer(){
    //int fd_error;
    //system("rm /tmp/fifo");

    /* redirect error fd to ERRORS file */
    /**
    if((fd_error = open(ERRORS, O_WRONLY)) == -1){
		write(2, "Error logging.\n", 19);
		return -1;
	} else {
		dup2(fd_error, 2);
		close(fd_error);
	}*/

	
    return 0;
}


int initCommunication(){
    /* get clients pid */


    /*openning fifos to comunicate with client */
    /*
    sprintf(s2cPath, "%s%d", S2CFIFO, clientPid);
    if((ser2cli = open(s2cPath, O_WRONLY)) == -1){
        perror("open ser2cli fifo");
        return -1;
    }*/
    c2sPath = malloc(sizeof(char) * 64);
    sprintf(c2sPath, "tmp/fifo%ld", clientPid);
    printf("%s\n", c2sPath);
    if((cli2serv = open(c2sPath, O_RDONLY)) == -1){
        perror("open cli2serv fifo");
        return -1;
    }

    return 0;
}

int main(int argc, char* argv[]){
    int fd, fifo;
    int bytes_read;
    char buf[MAX_READER];

    //if(initServer() == -1) perror("init server");

    if(mkfifo(FIFO, 0666) == -1) perror("server fifo");

    while(1){
        /* while there is no client to write on my fifo reading is blocked */
        if((fifo = open("tmp/fifo", O_RDONLY)) == -1)
            perror("open");

        initCommunication();

        /*
        while((bytes_read = read(fifo, buf, MAX_READER)) > 0){
            write(1, buf, bytes_read);
        }*/

        close(fifo);
    }

    unlink(FIFO);
    close(fd);

    return 0;
}