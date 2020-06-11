#include "utilities.h"

int fifo, clififo;
char* path;
char serverPath[128];

void sendMessage(char* message, int size) {
    int fd = open(serverPath,O_WRONLY);
    if(write(fd,message,size) == -1) perror("Comunicação com o servidor falhou");
    close(fd);
}

void help(){
    write(1, "tempo-inactividade segs\n", 24);
    write(1, "tempo-execucao segs\n", 20);
    write(1, "executar 'p1 | p2 ... | pn'\n", 28);
    write(1, "listar\n", 7);
    write(1, "terminar tarefa\n", 16);
    write(1, "historico\n", 10);
    write(1, "output tarefa\n", 14);
}

int parser(char** input, int size){
    char message[OUT];
    memset(message, '\0', OUT);

    if(size==1){
        if(strcmp(input[0], "-l")==0 || strcmp(input[0], "listar")==0){
            strcpy(message, "l");
        }else if(strcmp(input[0], "-r")==0 || strcmp(input[0], "historico")==0){
            strcpy(message, "r");
        }else if(strcmp(input[0], "-h")==0 || strcmp(input[0], "ajuda")==0){
            help();
        }else return -1;
    }else if(size == 2){
        if(strcmp(input[0], "-i")==0 || strcmp(input[0], "tempo-inactividade")==0){
            strcpy(message, "i ");
        }else if(strcmp(input[0], "-m")==0 || strcmp(input[0], "tempo-execucao")==0){
            strcpy(message, "m ");
        }else if(strcmp(input[0], "-t")==0 || strcmp(input[0], "terminar")==0){
            strcpy(message, "t ");
        }else if(strcmp(input[0], "-o")==0 || strcmp(input[0], "output")==0){
            strcpy(message, "o ");
        }else return -1;
        strcat(message, input[1]);
        printf("%s", message);
    }else if(strcmp(input[0], "-e")==0 || strcmp(input[0], "executar")){

    }else return -1;
     
    sendMessage(message, strlen(message));

    for(int i = 0; input[i]; i++) free(input[i]);
	free(input);

    return 0;
}

int initCommunication(){
    strcpy(serverPath, FIFOS);

    path = malloc(sizeof(char) * PATH_SIZE);
	memset(path, '\0', 64);
	snprintf(path, 64, "%s%ld", FIFOS, (long) getpid());

    if(mkfifo(path, 0666) == -1) {
        perror("client fifo");
        return -1;
    }

    if((fifo = open(serverPath, O_WRONLY)) == -1){
        perror("open");
        return -1;
    }

    /* sending my fifo's path to server */
    write(fifo, path, strlen(path));

    return 0;
}

int main(int argc, char* argv[]){
    int bytes_read;
    char buf[BUFFER_SIZE];
    int res = 0;;

    if(initCommunication() == -1) perror("Communication");

    
     
    if(fork() == 0){
        /* writes to server fifo */ 
        while(1){
            if((fifo = open(serverPath, O_WRONLY)) == -1) perror("open");

            while((bytes_read = readln(0, buf, BUFFER_SIZE)) > 0){
                /*parse input*/
                buf[res-1] = '\0';
                int size;
                char** wrds = words(buf, &size);
                if(parser(wrds, size) == -1) write(1, "Input Inválido!\n", 17);
            }
            close(fifo);
        }
        
    }

    if(fork() == 0){
        /* reads from myfifo */
        /*inside while*/
        
        while(1){
            if((clififo = open(path, O_RDONLY)) == -1) perror("open clififo");


            while((res = read(clififo, buf, BUFFER_SIZE)) > 0){
                write(1, buf, res);
            }

            close(clififo);
        }
    }

    for(int i=0; i<2; i++) wait(NULL);

    close(clififo);
    unlink("clififo");
    close(fifo);
    
    return 0;
}