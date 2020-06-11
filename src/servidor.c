#include "utilities.h"

#define MAX_READER 1024

pid_t clientPid;
int clientfifo, serverFifo;
char serverPath[128];
char clientPath[128];


void parseMessage(char* command){
    int size; 
    char** lines = words(command, &size);

    switch(* lines[0]){
        case 'i':
            break;
        case 'o':
            break;
        default : 
            printf("defaulrt");
            break;
    }
}

int initServer(){
    /* signals */

    strcpy(serverPath, FIFOS);
    if(mkfifo(serverPath, 0666) == -1){
        perror("fifo"); 
        return -1;
    }

    return 0;
}

void clientCommunication(){
    int n = read(serverFifo, clientPath, 128);
    clientPath[n] = '\0';

    clientPid = getPidFromPath(clientPath);
}

int main(int argc, char* argv[]){
    int bytes_read;
    char buf[MAX_READER];
    int res;

    if(initServer() == -1) perror("init server");

    while(1){
        if((serverFifo = open(serverPath, O_RDONLY)) == -1) perror("open");
        
        clientCommunication();

        while((res = readln(serverFifo, buf, MAX_READER)) > 0){
            /* buf comes with \n*/
            printf("%s", buf);
            parseMessage(buf);
        }
        
        close(serverFifo);
    }

    unlink(serverPath);

    return 0;
}