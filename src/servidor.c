#include "utilities.h"

#define MAX_READER 1024

pid_t clientPid;
int clientfifo, serverFifo;
char serverPath[128];
char clientPath[128];


int n_tasks = 0;
int max_inativity = 0;
int max_execution = 0;
size_t currentpos = 0; //last position written on log file
CircularArray* ca;




void parseMessage(char* command){
    int size; 
    char** lines = words(command, &size);

    
    pid_t child;
    if((child = fork()) == 0){
        switch(* lines[0]){
            case 'i':
                max_inativity = atoi(lines[1]);
                break;
            case 'm':
                max_execution = atoi(lines[1]);
                break;
            case 'e': /* execute task */;
                //executeTask(id_task, line[1]);
                break;
            case 'l': /* list tasks */
                //showExecutingTasks();
                break;
            case 't': /* end taks */
                //endTask(lines[1]);
                break;
            case 'r': /* history of tasks */
                break;
            case 'o':
                break;
        }
        _exit(0);
    }

    if(*lines[0] == 'e'){
        int id_task = n_tasks +1;
        insertCA(ca, id_task, command); 
    }
}

int sendToClient(char* message, int size){
    if((clientfifo = open(clientPath, O_WRONLY)) == -1) perror("client fifo");

    if(write(clientfifo, message, size) == -1) perror("error writing to client");

    close(clientfifo);
    return 0;
}

void clientCommunication(){
    int n = read(serverFifo, clientPath, 128);
    clientPath[n] = '\0';
    clientPid = getPidFromPath(clientPath);
}

int initServer(){
    /* signals */

    ca = initCA(CACHE);

    n_tasks = countTaks(&currentpos);
    printf("nr_tasks %d\n", n_tasks);

    strcpy(serverPath, FIFOS);
    if(mkfifo(serverPath, 0666) == -1){
        perror("fifo"); 
        return -1;
    }

    return 0;
}

int main(int argc, char* argv[]){
    char buf[MAX_READER];
    int res;

    if(initServer() == -1) perror("init server");

    while(1){
        if((serverFifo = open(serverPath, O_RDONLY)) == -1) perror("open");
        
        clientCommunication();

        while((res = read(serverFifo, buf, MAX_READER)) > 0){
            write(1, buf, res);
            parseMessage(buf);
            //sendToClient(buf, strlen(buf));
        }
        
        close(serverFifo);
    }

    unlink(serverPath);

    return 0;
}