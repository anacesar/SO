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

int countTaks(size_t* currentpos){
	int res=0;
	int fd= open(LOGIDX, O_RDONLY | O_CREAT, 0666);
    if(fd == -1) return -1;
	
    Task t;
	while(read(fd, &t, sizeof(struct task)) > 0) res++;
    *currentpos = t.beginning + t.size;
	return res;
}

Task new_Task(int id, char* comm){
	Task task; //= malloc(sizeof(struct task));
	task.id = id;
    task.beginning=0;
    task.size=0;
    task.state = 3;
    strcpy(task.command, comm);

	return task;
}

CircularArray* initCA(int size){
    CircularArray *c = malloc(sizeof(struct circularArray));
    c->size = size;
    c->currentPos = 0;
    c->tasks = malloc(sizeof(struct task) * c->size);
    return c;
}

/* Regular insertion */
void insertCA(CircularArray *c, int id, char* command){
    if(c->currentPos == c->size) c->currentPos = 0;
    Task task = new_Task(id, command);
    c->tasks[c->currentPos] = task;
}

void endTask(pid_t pid){

}


void freeCircularArray(CircularArray *c){
    free(c->tasks);
    free(c);
}

/* Control options:
0 -> Binary mode
1 -> Readable mode */
/*
void flushCircularArray(CircularArray *c, int control){
    char out[256];
    for(int i = 0; i < c->size ; i++){
        if(control){
            sprintf(out, "Code: %d\nQuantity: %d\nTotal: %f\n", c->entries[i].artCode, c->entries[i].quantity, c->entries[i].total);
            write(1, out, strlen(out));
            memset(out, '\0', 256);
        } else {
            struct venda v;
            v.code = c->entries[i].artCode;
            v.quantity = c->entries[i].quantity;
            v.total = c->entries[i].total;
            write(1, &v, sizeof(struct venda));
        }
    }
}
*/