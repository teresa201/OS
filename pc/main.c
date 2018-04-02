#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>

extern char **environ;
#define BUFSIZE 4

pthread_mutex_t the_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condc, condp = PTHREAD_COND_INITIALIZER;
char** buffer[BUFSIZE];
int status = 1;

//next free space in buffer
int open = 0;
//next avail number in buffer
int last = 0;
//items in buffer
int count = 0;

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


int launch(char** line){
    system(*line);
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

int frand(char ** args){

    char* fName = args[1];
    char* n = args[2];
    int num = atoi(n);

    FILE* f = fopen(fName,"w");
    srand((unsigned)time(0));
    int k;
    for(k = 0; k < num; k++){
        fprintf(f,"%d \n", rand() % 100);
    }
    fclose(f);
    return 1;
}

int cmpfunc(const void * a, const void *b){
    return (*(int*)a - *(int*)b);
}



int fsort(char ** args){
    char* fName = args[1];
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
    FILE* fp = fopen(fName, "r+");
    if(fp == NULL){
        printf("Could not open file \n");

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

   //quicksort the values
   int numInVector = pos;
   int j;
   qsort(values, numInVector, sizeof(int), cmpfunc);
   fprintf(fp,"Sorted List of Values: \n");
   for(j = 0; j < numInVector; j++){
       fprintf(fp, "%d \n", values[j]);
   }
   fclose(fp);

    return 1;
}


int execute(char** chop){
    int k;

    if(chop == NULL){
        return 1;
    }

    //if internal function run its function
    for(k = 0; k < 6; k++){
            if(strcmp(chop[0], internal[k]) == 0){
            return (*internal_func[k])(chop);
        }
    }

    //if not an internal function run it
    return launch(chop);
}


void* producer(void *ptr){
    char *input;
    char *inSaved;
    char **args;
    int status;

    do{
    //get input from user
    printf("> ");
    input = readLine();
    args = parseLine(input);
    //lock buffer
    pthread_mutex_lock(&the_mutex);

    while(count == BUFSIZE) //while buffer is full wait
    {
        pthread_cond_wait(&condp, &the_mutex);
    }
   //add to buffer
    buffer[open] = args;
    open = (open + 1) % BUFSIZE;
    count++;
    //unlock and alert consumer
    pthread_cond_signal(&condc);
    pthread_mutex_unlock(&the_mutex);
    }while(true);
    pthread_exit(NULL);

}





void* consumer(void * ptr){
    char** off;
    status = 1;
    while(status){
    pthread_mutex_lock(&the_mutex);

    while(count == 0) //wait if nothing to consume
    {
        pthread_cond_wait(&condc, &the_mutex);
    }
    //get from buffer
    off = buffer[last];
    last = (last + 1) % BUFSIZE;
    count--;
    //run command
    status = execute(off);
    //unlock and alert producer
    pthread_cond_signal(&condp);
    pthread_mutex_unlock(&the_mutex);
    }
    pthread_exit(NULL);

    return NULL;

}



int main(int argc, char *argv[])
{
    char *input;
    char *inSaved;
    char **args;
    int status;

    pthread_t p, c;

    //initialize condition and mutex variables
    pthread_mutex_init(&the_mutex, NULL);
    pthread_cond_init(&condc, NULL);
    pthread_cond_init(&condp, NULL);


    //create threads
    pthread_create(&p, NULL, producer, NULL);
    pthread_create(&c, NULL, consumer, NULL);

    pthread_join(p, NULL);
    pthread_join(c, NULL);

    return 0;
}





