#include "argus.h"

int fifo, clififo;
char* path;
char serverPath[128];

void sendMessage(char* message, int size) {
    fifo = open(serverPath,O_WRONLY);
    if(write(fifo,message,size) == -1) perror("Server communication");
    close(fifo);
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
        if(isNumber(input[1])){
            if((strcmp(input[0], "-i")==0 || strcmp(input[0], "tempo-inactividade")==0) && isNumber(input[1])){
                sprintf(message, "%s %d", "i", atoi(input[1]));
            }else if((strcmp(input[0], "-m")==0 || strcmp(input[0], "tempo-execucao")==0) && isNumber(input[1])){
                sprintf(message, "%s %d", "m", atoi(input[1]));
            }else if((strcmp(input[0], "-t")==0 || strcmp(input[0], "terminar")==0) && isNumber(input[1])){
                sprintf(message, "%s %d", "t", atoi(input[1]));
            }else if((strcmp(input[0], "-o")==0 || strcmp(input[0], "output")==0) && isNumber(input[1])){
                sprintf(message, "%s %d", "o", atoi(input[1]));
            }else return -1;
        }else if(strcmp(input[0], "-e")==0 || strcmp(input[0], "executar")==0){
            sprintf(message, "%s %s", "e", input[1]); 
        }else return -1;
    }else return -1;

    sendMessage(message, strlen(message));

    return 0;
}

void closeClient(){
    close(clififo);
    close(fifo);
    unlink(path);
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
    char buf[MAX_READER];
    int res = 0;;

    if(initCommunication() == -1) perror("Communication");

	if(argc > 1){
		parser(argv+1, argc-1);
        if((clififo = open(path, O_RDONLY)) == -1) perror("open clififo");
        while((res = read(clififo, buf, MAX_READER))<=0);
        write(1, buf, res);
        PUT_LINE;
        closeClient();
        return 0;
	}

    if(fork() == 0){
        /* writes to server fifo */ 
        while(1){
            if((fifo = open(serverPath, O_WRONLY)) == -1) perror("open");

            while((bytes_read = readln(0, buf, MAX_READER)) > 0){
                /*parse input*/
                buf[bytes_read-1] = '\0';
                int size;
                char** wrds = words(buf, &size);
                if(parser(wrds, size) == -1) write(1, "Input Inválido!\n", 17);
			
    			for(int i = 0; wrds[i]; i++) free(wrds[i]);
				free(wrds);
			}

            close(fifo);
        }
    }

    if(fork() == 0){
        /* reads from myfifo */
        while(1){
            if((clififo = open(path, O_RDONLY)) == -1) perror("open clififo");

            while((res = read(clififo, buf, MAX_READER)) > 0){
                write(1, buf, res);
				PUT_LINE;
            }

            close(clififo);
        }
    }

    for(int i=0; i<2; i++) wait(NULL);

    closeClient();
    
    return 0;
}