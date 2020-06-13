#include "utilities.h"
#include "server_api.h"

#define MAX_READER 1024

pid_t clientPid;
int clientfifo, serverFifo;
char serverPath[128];
char clientPath[128];
int outFile;

/*used by processes running tasks*/
int* pids;
int npids;
int timeout = 0;


int n_tasks = 0;
int max_inativity = 0;
int max_execution = 0;
int currentpos = 0; //last position written on log file
CircularArray* ca;


void sigquit_handler(int signum){

}

void max_exec_handler(int signum){
    for (int i = 0; i < npids; i++){
		if(pids[i] > 0){
		    kill(pids[i], SIGKILL);
		}
	}
    timeout = 1;
}


int sendToClient(char* message, int size){
    if((clientfifo = open(clientPath, O_WRONLY)) == -1) perror("client fifo");

    if(write(clientfifo, message, size) == -1) perror("error writing to client");

    close(clientfifo);
    return 0;
}


// 0--> success
// -1 --> an error as occurred 
// -2 --> timeout (max_execution) 
int pipeTask(Task* task){
    char* commands[MAXCOMMANDS];
    int n_commands = getcommands(task->command, commands);

    /* send task id to client*/
    char message[OUT];
    memset(message, '\0', OUT);
    sprintf(message, "nova tarefa #%d", task->id);
    sendToClient(message, strlen(message));

    /* build the pipes for executing diferent commands */
    int pipes[n_commands-1][2]; //n commands --> n-1 pipes 
    
    int i;
    /* init array of pids */
    npids = n_commands;
    pids = (int* ) malloc(npids * sizeof(int));
    for(i=0; i<npids; i++) pids[i] = -1;

    int mainPid = getpid();
    int pid;

    if(signal(SIGALRM, max_exec_handler) == SIG_ERR) perror("alarm error");

    // n sons --> n_commands  
    // son 0 --> task 1 ...
    if(max_execution) alarm(max_execution);

    for(i=0; i<n_commands /*&& task->state == 1*/; i++){
        //last son
        if(i==n_commands-1){
            pid = fork();
            switch (pid){
                case -1:
                    perror("fork n-1");
                    return -1; //exit(-1) when we dont want the process to die 
                case 0: //son n-1
                    // son needs to redirect stdin to the reading extremity of the pipe created by the previous son (n-2)
                    if(n_commands > 1){
                        dup2(pipes[i-1][0], 0);
                        close(pipes[i-1][0]);
                    }
                    /* redirect output to temp file if task is executing*/
                    int out = open(outputFile(mainPid), O_CREAT | O_WRONLY, 0666);
                    if(out == -1) perror("opening output file to write output");
                    dup2(out, 1);
                    close(out);

                    exec_command(commands[i]);
                    _exit(-1); //only get where when theres an error with exec
                    break;
                default:
                    close(pipes[i-1][0]);
                    pids[i] = pid;
                    break;
            }
        }else{
            if(pipe(pipes[i]) == -1) {
                perror("pipe");
                return -1;
            }
            if(i == 0){
                switch ((pid = fork())){
                    case -1:
                        perror("fork 0");
                        return -2; //exit(-1) when we dont want the process to die 
                    case 0: //son n-1
                        close(pipes[i][0]);
                        // son needs to redirect stdout to the writing extremity of the pipe he created and close reading extreme
                        dup2(pipes[i][1], 1);
                        close(pipes[i][1]);

                        exec_command(commands[i]);
                        _exit(-1); //only get where when theres an error with exec
                        break;
                    default:
                        //dad closing the reading extremity prevents for his childs not having to do it 
                        close(pipes[i][1]);
                        pids[i] = pid;
                        break;
                } 
            }else{
                switch ((pid = fork())){
                    case -1:
                        perror("fork 1.. n-2");
                        return -1; //exit(-1) when we dont want the process to die 
                    case 0: //son n-1
                        // son needs to redirect stdin to the reading extremity of the pipe created by the previous son (i-1)
                        dup2(pipes[i-1][0], 0);
                        close(pipes[i-1][0]);

                        //also needs to redirect stdout to writing extremity of the pipe he created
                        dup2(pipes[i][1], 1);
                        close(pipes[i][1]);

                        close(pipes[i][0]);

                        exec_command(commands[i]);
                        _exit(-1); //only get where when theres an error with exec
                        break;
                    default:
                        close(pipes[i-1][0]);
                        close(pipes[i][1]);
                        pids[i] = pid;
                        break;
                } 
            }
        }
    }

    /* something happened to the task while trying to execute */
    if(task->state != 1) return -3; 

    if(timeout) return -2;
    
    int status = 0;
    for(int i=0 ; i<n_commands; i++) wait(&status);

    return 0;
}

void executeTask(char* command){
    /*task id*/
    int id_task = n_tasks+1;
    n_tasks++;
    
    Task* task = insertCA(ca, id_task, command);
    printf("%d\n", task->id);
    printf("%s\n", task->command);

    int status, res, fd, size = 0, exit;
    pid_t child = fork();
    switch(child){
        case -1: 
            finishTask(task, 5, 0, 0);
            break;
        case 0: /* son executes task*/
            exit = pipeTask(task);
            _exit(exit);
            break;
        default:
            waitpid(child, &status, 0);
            printf("status %d\n", (WEXITSTATUS(status)));
            switch((WEXITSTATUS(status))){
                case 0: /* task ended correctly*/
                    fd = open(outputFile(child) , O_RDONLY);
                    if(fd == -1) perror("opening output file to read");
                    char buffer[MAX_READER];
                    while((res = read(fd, buffer, MAX_READER))>0){
                        size += res;
                        write(outFile, buffer, res);
                    }
                    finishTask(task, 0, currentpos, size);
                    currentpos += size;
                    printf("current pos after writing %d\n", currentpos);
                    break;
                case -1: /* error executing */
       
                    break;
                case -2: /* max_exec timeout */
                    finishTask(task, 4, 0, 0);
                    break;
                default: /* something happened to the task */
                    break;
            }
            break;
    }

}

void listExecutingTasks(){
    /* take this fork when doing fork to execute tasks */
    if(fork()==0){
        int i = 0;
        Task* task = ca->tasks[i];
        while(task!=NULL && i<ca->size){
            char out[256];
            memset(out, '\0', 256);

            sprintf(out, "#%d: %s", task->id, task->command);
            sendToClient(out, strlen(out));
            task = ca->tasks[++i];
        }
        _exit(0);
    }
}


void parseMessage(char* command){
    int size; 
    char** lines = words(command, &size);

    if(* lines[0] == 'e'){
        executeTask(command + 2);
    }else if(* lines[0] == 'l'){
        listExecutingTasks();
    }
    else{
        switch(* lines[0]){
            case 'i':
                max_inativity = atoi(lines[1]);
                break;
            case 'm':
                max_execution = atoi(lines[1]);
                break;
        }
    }
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
    printf("current_pos %d\n", currentpos);

    strcpy(serverPath, FIFOS);
    if(mkfifo(serverPath, 0666) == -1){
        perror("fifo"); 
        return -1;
    }
    /* open file to write outputs */
    outFile = open(strdup(LOG), O_WRONLY | O_CREAT | O_APPEND, 0666);
    if(outFile == -1) perror("error openning log file");

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
            parseMessage(buf);
            //sendToClient(buf, res);
        }
        
        close(serverFifo);
    }

    unlink(serverPath);
    close(outFile);

    return 0;
}