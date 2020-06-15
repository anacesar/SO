#include "argus.h"

pid_t clientPid;
int clientfifo, serverFifo;
char serverPath[128];
char clientPath[128];
int outFile;

/*used by processes running tasks*/
int* pids;
int npids;
int timeout = 0;
int interrupted = 0;

int n_tasks = 0;
int max_inactivity = 0;
int max_execution = 0;
int currentpos = 0; //last position written on log file
CircularArray* ca;

int outputSize(int pid){
    int res, size = 0;
    int fd = open(outputFile(pid) , O_RDONLY);
    if(fd == -1) {
        /*an error ocurred in the execution of the task */
        return 0;
    }
    char buffer[MAX_READER];
    while((res = read(fd, buffer, MAX_READER))>0){
        size += res;
        if(write(outFile, buffer, res) == -1) perror("writing to output file");
    }

    currentpos += size;
    return size;
}

void sigchild_handler(int signum) {
  int pid = -1, status;
  while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
      if(WIFEXITED(status) && WEXITSTATUS(status)!=5){
        Task* task = findTask(ca, pid);
        finishTask(task, WEXITSTATUS(status), currentpos, outputSize(pid));
      } 
  }
}

void max_exec_handler(int signum){
    for (int i = 0; i < npids; i++){
		if(pids[i] > 0){
		    kill(pids[i], SIGKILL);
		}
	}
    if(signum == SIGALRM) timeout = 1;
    if(signum == SIGINT) interrupted = 1;
}

void max_inactivity_handler(int signum){
    /* exit with exitcode 1 */
    _exit(1);
}

int sendToClient(char* message, int size){
    if((clientfifo = open(clientPath, O_WRONLY)) == -1) perror("client fifo");

    if(write(clientfifo, message, size) == -1) perror("error writing to client");

    close(clientfifo);
    return 0;
}

// 0--> success
// -1 --> an error as occurred 
// 3--> timeout (max_inactivity)
// 4 --> timeout (max_execution) 
int pipeTask(char* command){
    char* commands[MAXCOMMANDS];
    int n_commands = getcommands(command, commands);

    /* build the pipes for executing diferent commands */
    int pipes[n_commands-1][2]; //n commands --> n-1 pipes 
    int i;
    
    /* init array of pids */
    npids = n_commands;
    pids = (int* ) malloc(npids * sizeof(int));
    for(i=0; i<npids; i++) pids[i] = -1;

    int mainPid = getpid();

    if(signal(SIGALRM, max_exec_handler) == SIG_ERR) perror("pipeTask-alarm error");
    //to deal with interrupction
    if(signal(SIGINT, max_exec_handler) == SIG_ERR) perror("pipeTask-alarm error");

    if(max_execution) alarm(max_execution);

    // n sons --> n_commands  
    // son 0 --> task 1 ...
    for(i=0; i<n_commands /*&& task->state == 1*/; i++){
        int pid = -1;
        //last son
        if(i==n_commands-1){
            pid = fork();
            switch (pid){
                case -1:
                    perror("fork n-1");
                    return -1; //exit(-1) when we dont want the process to die 
                case 0: //son n-1
                    if(signal(SIGALRM, max_inactivity_handler) == SIG_ERR) perror("fork alarm");
                    if(max_inactivity) alarm(max_inactivity);
                    // son needs to redirect stdin to the reading extremity of the pipe created by the previous son (n-2)
                    if(n_commands > 1){
                        dup2(pipes[i-1][0], 0);
                        close(pipes[i-1][0]);
                    }
                    
                    //sleep(10); /*to test inativity and max exec*/

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
                pid = fork();
                switch (pid){
                    case -1:
                        perror("fork 0");
                        return -2; //exit(-1) when we dont want the process to die 
                    case 0: //son 0
                        signal(SIGALRM, max_inactivity_handler);
                        if(max_inactivity) alarm(max_inactivity);
                
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
                pid = fork();   
                switch (pid){
                    case -1:
                        perror("fork 1.. n-2");
                        return -1; //exit(-1) when we dont want the process to die 
                    case 0: 
                        signal(SIGALRM, max_inactivity_handler);
                        if(max_inactivity) alarm(max_inactivity);

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

    int status = 0;
    for(int i=0 ; i<n_commands; i++){
        wait(&status);
        if(WIFEXITED(status) && WEXITSTATUS(status) == 1){
            /* pipe with pid pid ended by max inactivity --> kill all the others */
			max_exec_handler(0);
            return 3;
		}
    }

    if(interrupted) return 2;
    if(timeout) return 4;

    /* task was complete */
    return 0;
}

void executeTask(char* command){
    /*task id*/
    int id_task = n_tasks+1;
    n_tasks++;

    Task* task;
    int child = fork();
    switch(child){
        case -1: 
            perror("fork error");
            break;
        case 0: /* son executes task*/
            if(signal(SIGCHLD, SIG_DFL) == SIG_ERR) perror("executeTask: signal error");
            close(outFile);
            _exit(pipeTask(command));
            break;
        default:
            task = insertCA(ca, id_task, command, child);
            /* send task id to client*/
            char message[OUT];
            memset(message, '\0', OUT);
            sprintf(message, "nova tarefa #%d", task->id);
            sendToClient(message, strlen(message));

            break;         
    }
}

void listExecutingTasks(){
    int i = 0;
    while(ca->tasks[i] && i<ca->size){
        if(ca->tasks[i]->state == 1){
            char out[256];
            memset(out, '\0', 256);
            sprintf(out, "#%d: %s", ca->tasks[i]->id, ca->tasks[i]->command);
            sendToClient(out, strlen(out));
        }
        i++;
    } 
}

void terminateTask(int task_id){
    Task* task = findTaskID(ca, task_id);
    if(task){
        /* sending signal to stop executition*/
        kill(task->pid, SIGINT);
    }
}

void history(){
    int i = 0;
    while(ca->tasks[i] && i<ca->size){
        char out[256];
        memset(out, '\0', 256);
        switch(ca->tasks[i]->state){
            case 0: 
                sprintf(out, "#%d, concluida: %s", ca->tasks[i]->id, ca->tasks[i]->command);
                sendToClient(out, strlen(out));
                break;
            case 2:
                sprintf(out, "#%d, terminada: %s", ca->tasks[i]->id, ca->tasks[i]->command);
                sendToClient(out, strlen(out));
                break;
            case 3:
                sprintf(out, "#%d, max inatividade: %s", ca->tasks[i]->id, ca->tasks[i]->command);
                sendToClient(out, strlen(out));
                break;
            case 4:
                sprintf(out, "#%d, max execução: %s", ca->tasks[i]->id, ca->tasks[i]->command);
                sendToClient(out, strlen(out));
                break;
            default:
                break;
        }
        i++; 
    } 
}

void outputTask(int id){
    /* no output for task that doesnt exist*/
    if(id<=n_tasks){
        int logidx = open(LOGIDX, O_RDONLY);
        lseek(logidx, (id-1)*sizeof(struct task), SEEK_SET);

        /* get task struct in log.idx file*/
        Task task;
        read(logidx, &task, sizeof(struct task));

        if(task.size>0){
            int log = open(LOG, O_RDONLY);
            lseek(log, task.beginning, SEEK_SET);
            char message[task.size];
            read(log, message, task.size);
            sendToClient(message, task.size);
            close(log);
        }
        close(logidx);
    }
}

void parseMessage(char* command){
    int size; 
    char** lines = words(command, &size);
   
    if(* lines[0] == 'i')  max_inactivity = atoi(lines[1]);
    else if(* lines[0] == 'm') max_execution = atoi(lines[1]); 
    else if(* lines[0] == 'e'){
        executeTask(command + 2);
    }else{
        switch(fork()){
            case -1:
                perror("parseMessage - fork");
                break;
            case 0: /* create son to execute some commands */
                switch (*lines[0]){
                case 'l': /* list executing tasks*/
                    listExecutingTasks();
                    break;
                case 't': /* end a task */
                    terminateTask(atoi(lines[1]));
                    break;
                case 'r': /* history of completed tasks */
                    history();
                    break;
                case 'o': /* output from a task */
                    outputTask(atoi(lines[1]));
                    break;
                default:
                    break;
                }
                _exit(5); //5-> for server to do nothing in SIGCHILD handler 
            default: /* dad keeps on reading from client */
                break;
        }
    }

    for(int i=0;i<2;i++) free(lines[i]);
    free(lines);
}

void clientCommunication(){
    int n = read(serverFifo, clientPath, 128);
    clientPath[n] = '\0';
    clientPid = getPidFromPath(clientPath);
}

int initServer(){
    /* signals */
    if(signal(SIGCHLD, sigchild_handler) == SIG_ERR) printf("initserver: sigchild error");
    ca = initCA(CACHE);
    n_tasks = countTaks(&currentpos);

    strcpy(serverPath, FIFOS);
    if(mkfifo(serverPath, 0666) == -1){
        perror("fifo"); 
        return -1;
    }
    /* open file to write outputs */
    outFile = open(LOG, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if(outFile == -1) perror("error openning log file");
    lseek(outFile, currentpos, SEEK_SET);

    return 0;
}


int main(int argc, char* argv[]){
    char* buf = malloc(MAX_READER);
    int res;

    if(initServer() == -1) perror("init server");

    while(1){
        if((serverFifo = open(serverPath, O_RDONLY)) == -1) perror("open");
        
        clientCommunication();

        while((res = read(serverFifo, buf, MAX_READER)) > 0){
            parseMessage(buf);
            memset(buf, '\0', MAX_READER);
        }
        
        close(serverFifo);
    }

    unlink(serverPath);
    close(outFile);

    return 0;
}