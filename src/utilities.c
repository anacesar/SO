#include "utilities.h"

/* returns how many bytes until a newline */
ssize_t readln(int fd, char *line, size_t size){
    int n = read(fd, line, size);
    int i = 0;
    if(n > 0){
        for(; i<size && line[i] != '\n'; i++);
        lseek(fd, -(n-i-1), SEEK_CUR);
    //to get '\n'
    i++; 
    }

    return i;
}

void sendMyPid(int fifo){
    char pid[20];
    sprintf(pid, "%d", getpid());

    write(fifo, pid, strlen(pid));
}

pid_t getPidFromPath(char* path){
    char pid[10];
    int i, ret;

    /* Find the first digit */
    for(i = 0; path[i] <= 47 || path[i] >= 58; i++) 
        ;

    /* Copy all digits into string pid */
    for(int j = 0; path[i]; j++)
        pid[j] = path[i++];

    /* Convert to pid_t */
    ret = atoi(pid);

    return (pid_t) ret;
}

char** words(const char* line, int *size){
    char* input = strdup(line);

    /* to remove quotes from command: ’cut -f7 -d: /etc/passwd | uniq | wc -l’ -. cut -f7 -d: /etc/passwd | uniq | wc -l */
    if(*line == 'e' || line[1] == 'e'){
        char** words = malloc(3 * sizeof(char*));
        words[0] = strdup(strtok(input, " "));
        words[1] = strdup(strtok(NULL, "'"));
        words[2] = NULL;

        (*size) = 2;
        return words;
    }

    int word_count = 1;
    while (*line) {
        if (*line == ' '){
            while (*line && *line == ' ') {
                line++;
            }
            word_count++;
        }else {
            line++;
        }
    }

    char** words = malloc((word_count + 1) * sizeof(char*));
    int word = 0;
    for(char* token = strtok(input, " "); token; token = strtok(NULL, " ")) {
        words[word++] = strdup(token);
    }
    words[word] = NULL;
    (*size) = word;
    free(input);
    return words;
}


char* outputFile(int pid){
    char* path = malloc(sizeof(char) * PATH_SIZE);
	memset(path, '\0', 64);
	snprintf(path, 64, "%s%d", TMP, pid);

    printf("%s\n", path);

    return path;
}

int countTaks(int* currentpos){
	int res=0;
	int fd= open(LOGIDX, O_RDONLY | O_CREAT);
    if(fd == -1) perror("openning log.idx file");

    Task task;
	while(read(fd, &task, sizeof(struct task)) > 0){
        *currentpos += task.size;
        res++;
    }

    close(fd);
	return res;
}

CircularArray* initCA(int sizee){
    CircularArray* ca = (CircularArray*) malloc(sizeof(struct circularArray));
    ca->currentPos = 0;
    ca->size = sizee;
    ca->tasks = (Task**) malloc(ca -> size * sizeof(Task *));
    for(int i = 0; i < ca->size; i++) {
        ca->tasks[i] = NULL;
    }
    return ca;
}

Task* insertCA(CircularArray* c, int id, char* command){
    if(c->currentPos == c->size) c->currentPos = 0;
    c->tasks[c->currentPos] = malloc(sizeof(struct task));
    c->tasks[c->currentPos]->id = id;
    c->tasks[c->currentPos]->beginning = 0;
    c->tasks[c->currentPos]->size = id;
    c->tasks[c->currentPos]->state = 1;
    strcpy(c->tasks[c->currentPos]->command, command);
    return c->tasks[c->currentPos++];
}


int finishTask(Task* task, int status, int init, int size){
    int fd = open(strdup(LOGIDX), O_WRONLY | O_CREAT, 0666);
    if(fd == -1) return -1;

    task->state = status;
    task->beginning = init;
    task->size = size;

    lseek(fd, ((task->id -1)* sizeof(struct task)), SEEK_SET);
    if(write(fd, task, sizeof(struct task))== -1) perror("finishTask");

    close(fd);

    return 0;
}

int terminateTask(int id){
    return 0;
}


/* Control options:
0 . Binary mode
1 . Readable mode */
/*
void flushCircularArray(CircularArray *c, int control){
    char out[256];
    for(int i = 0; i < c.size ; i++){
        if(control){
            sprintf(out, "Code: %d\nQuantity: %d\nTotal: %f\n", c.entries[i].artCode, c.entries[i].quantity, c.entries[i].total);
            write(1, out, strlen(out));
            memset(out, '\0', 256);
        } else {
            struct venda v;
            v.code = c.entries[i].artCode;
            v.quantity = c.entries[i].quantity;
            v.total = c.entries[i].total;
            write(1, &v, sizeof(struct venda));
        }
    }
}
*/



/* executes a certain command */
int exec_command(char* command){
    char* exec_args[MAXARGS];
    char* token; 
    int exec_ret = 0;
    int i = 0;

    token = strtok(command, " ");
    
    for(; token!=NULL ; token = strtok(NULL, " "), i++) {exec_args[i] = token; printf("%d %s", i, exec_args[i]);}

    exec_args[i] = NULL;

    exec_ret = execvp(exec_args[0], exec_args);
    return exec_ret;

}

/* gets all commands from a pipeline */
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

