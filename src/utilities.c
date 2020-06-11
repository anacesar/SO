#include "utilities.h"

struct task {
	int id;
	int beginnig; //where output beggins in log file
	int size; //size of output 
	char command[COMMAND_SIZE];
};


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
    size_t word_count = 1;
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
    size_t word = 0;
    for(char* token = strtok(input, " "); token; token = strtok(NULL, " ")) {
        words[word++] = strdup(token);
    }
    words[word] = NULL;
    (*size) = word;
    free(input);
    return words;
}
