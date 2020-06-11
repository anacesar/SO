#include "utilities.h"


char** words(const char* line, int *size){
    char* input = strdup(line);
    size_t word_count = 1;
    while (*line) {
        if (*line == ' ') {
            while (*line && *line == ' ') {
                line++;
            }
            word_count++;
        } else {
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


int mypopen(char* command, char* type) {
    int pfd[2];
    int size;
    char** argv = words(command, &size);

    if(pipe(pfd) == -1){
        perror("pipe");
        return -1;
    }

    if(!fork()){
        if(type[0] == 'r') dup2(pfd[1], 1);
        else if(type[0] == 'w') dup2(pfd[0], 0);
        else _exit(1);

        close(pfd[0]);
        close(pfd[1]);
        execvp(argv[0], argv);
        _exit(1);
    }

    if(type[0] == 'r'){
        close(pfd[1]);
        return pfd[0];
    }
    else if (type[0] == 'w'){
        close(pfd[0]);
        return pfd[1];
    }

    return -1;
}

/* bytes == -1 => Reads until newline */
/* buf == NULL => Doesn't read, only counts how many bytes until a newline */
/*ssize_t readln(int fildes, void *buf, size_t bytes) {
    ssize_t size = 0;
    char ch;
    char * buff = (char*) buf;

    while((size < bytes || bytes == -1) && read(fildes, &ch, 1) == 1) {
        if (ch == '\0')
            return size;

        if (buff != NULL) {
            buff[size++] = ch;
        } else size++;

        if (ch == '\n')
            return size;
    }

    return size;
}*/ 

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

char* setPidToPath(pid_t pid){
    char* path = (char* ) malloc(SIZE_MAX * sizeof(char));

   sprintf(path, "%d.txt", pid);

   return (char*) path;
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