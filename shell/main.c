#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
/*@author Teresa Yienger
Used genuse27
run: gcc -pthread shell.c
*/
extern char **environ;


char* readLine(){
    char* l = NULL;
    size_t bSize = 0;
    getline(&l, &bSize, stdin);
    return l;
}

char** parseLine(char* str){
    int TOK_BUFSIZE = 64;

    int bufSize = TOK_BUFSIZE, pos = 0;
    char** split = malloc(bufSize * sizeof(char*));

    if(!split){
        fprintf(stderr, "allocation error\n");
        exit(EXIT_FAILURE);
    }

    //split apart str into individual tokens
    char* part = strtok(str, " \t\r\n\a");
    while(part != NULL){
        split[pos] = part;
        //printf("%s ", split[pos]);
        pos++;

        if(pos > bufSize){
            bufSize += TOK_BUFSIZE;
            split = realloc(split, bufSize * sizeof(char*));
            if(!split){
                fprintf(stderr, "allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        part = strtok(NULL," \t\r\n\a" );
    }
    split[pos] = NULL;
    return split;

}

int launch(char **args, char* line){
    pid_t pid;
    pid_t wpid;
    int status;
    printf("Launch %s", line);
    pid = fork();
    if(pid == 0){
        system(line);
        exit(EXIT_FAILURE);
    }else if(pid < 0){
        printf("Error forking process \n");
    }else{
        do{
            wpid = waitpid(pid, &status, WUNTRACED);
        }while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}



//Function Declarations for internal commands
int clr(char ** args);
int dir();
int envir(char ** args);
int quit(char ** args);
int frand(char ** args);
int fsort(char ** args);

//List of internal commands
char* internal[] = {
    "clr",
    "dir",
    "environ",
    "quit",
    "frand",
    "fsort"
};

int(*internal_func[])(char**) = {
        &clr,
        &dir,
        &envir,
        &quit,
        &frand,
        &fsort
};

int clr(char ** args){
   system("clear");
   return 1;
}

int dir(){
  system("ls -al");
  return 1;
}

int envir(char ** args){
    char ** env = environ;
    while (*env) printf("%s\n", *env++);
    return 1;
}


int quit(char ** args){
     exit(0);
    return 0;
}

void* oFile(void* args){
    char** data = (char**)args;
    char* fName = data[1];
    char* n = data[2];
    int num = atoi(n);

    FILE* f = fopen(fName,"w");
    srand((unsigned)time(0));
    int k;
    for(k = 0; k < num; k++){
        fprintf(f,"%d \n", rand() % 100);
    }
    fclose(f);

}

int frand(char ** args){
    pthread_t t1;
    pthread_create(&t1,NULL,oFile, (void *)args);
    pthread_join(t1,NULL);
    return 1;
}

int cmpfunc(const void * a, const void *b){
    return (*(int*)a - *(int*)b);
}

void* rFile(void* args){
    char* fName = args;
    int k = 0;
    int pos = 0;
    int bufSize = 64;

    //create vector to store integers
    int * values = malloc(bufSize * sizeof(char*));
    if(!values){
        fprintf(stderr, "allocation error\n");
        exit(EXIT_FAILURE);
    }

    //open the file given
    FILE* fp = fopen(fName, "r");
    if(fp == NULL){
        printf("Could not open file \n");
        //return 1;
    }

    //extract the integers from the file and store in values vector
    fscanf(fp, "%d", &k);
    while(!feof(fp)){
        values[pos] = k;
        pos++;
        if(pos > bufSize){
            bufSize += 64;
            values = realloc(values, bufSize * sizeof(char*));
            if(!values){
                fprintf(stderr, "allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        fscanf(fp, "%d", &k);
    }
   fclose(fp);

   //quicksort the values
   int numInVector = pos;
   int j;
   qsort(values, numInVector, sizeof(int), cmpfunc);
   printf("Sorted List of Values: \n");
   for(j = 0; j < numInVector; j++){
       printf("%d \n", values[j]);
   }


}


int fsort(char ** args){
    pthread_t t2;
    char* fName = args[1];
    pthread_create(&t2,NULL,rFile, fName);
    //pthread_join(t2,NULL);
    return 1;
}

int execute(char** chop, char* line){
    int k;
     //Empty command entered
    if(chop[0] == NULL){
        return 1;
    }

    //if internal function run its function
    for(k = 0; k < 6; k++){
        if(strcmp(chop[0], internal[k]) == 0){
            return (*internal_func[k])(chop);
        }
    }

    //if not an internal function run it
    return launch(chop,line);
}

int main(int argc, char *argv[])
{
    char *input;
    char *inSaved;
    char **args;
    int status;

    do{
        printf("> ");
        input = readLine();

        //deep copy of input
        inSaved = malloc(sizeof(input));
        memcpy(inSaved, input, sizeof(input));

        args = parseLine(input);
        status  = execute(args, inSaved);

        free(input);
        free(inSaved);
        free(args);

    }while(status);

    return 0;
}




