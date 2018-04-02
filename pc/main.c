#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>

extern char **environ;
#define MAX 1000000000
#define BUFSIZE 4

pthread_mutex_t the_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t empty = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t full = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condc, condp = PTHREAD_COND_INITIALIZER;
char* buffer[BUFSIZE];
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

//int launch(char **args, char* line){
int launch(char* line){
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
    pthread_join(t2,NULL);
    return 1;
}

//int execute(char** chop, char* line){
int execute(char* line){
    int k;
     //Empty command entered
    /*if(chop[0] == NULL){
        return 1;
    }*/

    //if internal function run its function
   /* for(k = 0; k < 6; k++){
        if(strcmp(chop[0], internal[k]) == 0){
            return (*internal_func[k])(chop);
      */
    if(line == NULL){
        return 1;
    }

    //if internal function run its function
    for(k = 0; k < 6; k++){
        if(strcmp(line, internal[k]) == 0){
            return (*internal_func[k])(&line);
        }
    }


    //if not an internal function run it
    return launch(line);
    //return launch(chop,line);
}
void putinBuffer(char **input){
    printf("In  PPPP \n");
    pthread_mutex_lock(&the_mutex);
    // pthread_cond_signal(&condc);
    while(count == BUFSIZE) //while buffer is full wait
    {
        printf("Producer Waiting");
        pthread_cond_wait(&condp, &the_mutex);
        //printf("Producer Waiting");
       // pthread_mutex_lock(&full);
    }
    printf("ABOUT TO ADD \n");
    buffer[open] = *input;
   // printf("%s \n", *input);
    printf("Producer: %s \n", buffer[open]);
    //printf("%d \n", open);
    open = (open + 1) % BUFSIZE;
    count++;

    int t;
    for(t = 0; t < BUFSIZE; t++){
        printf("END: %d, %s \n", t, buffer[t]);
    }
    printf("Count: %d \n", count);
    printf("NextOpen: %d \n", open);
    printf("One to get: %d \n", last);
    //pthread_cond_signal(&condc);
    pthread_mutex_unlock(&the_mutex);
   pthread_cond_signal(&condc);
   // pthread_mutex_unlock(&empty);
   // pthread_yield();
    printf("Unlocked");

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

    //deep copy of input
  //  inSaved = malloc(sizeof(input));
  //  memcpy(inSaved, input, sizeof(input));

    args = parseLine(input);

    //put in buffer
    putinBuffer(args);
    }while(true);
    pthread_exit(0);
    //free(input);
   // free(inSaved);
   // free(args);
}

/*char* getFromBuffer(){
    printf("In CCCCCC \n");
    char* off;
    pthread_mutex_lock(&the_mutex);
    //pthread_cond_signal(&condp);
    while(count == 0) //wait if nothing to consume
    {
        printf("Consumer Waiting");
        pthread_cond_wait(&condc, &the_mutex);
        //pthread_mutex_lock(&empty);
    }
   // pthread_mutex_unlock(&the_mutex);
    printf("CONSUMER: read buffer \n");

   // pthread_mutex_lock(&the_mutex);
    printf("Last %d, %s \n",last, buffer[0]);
    off = buffer[last];
    last = (last + 1) % BUFSIZE;
    count--;
    pthread_cond_signal(&condp);
    //printf("Consumer got %s \n", retrived);
    pthread_mutex_unlock(&the_mutex);
   // pthread_mutex_unlock(&full);
   // pthread_yield();
    //printf("Unlocked");
    return off;

}*/

void* consumer(void * ptr){
    printf("In CCCCCC \n");
    char* off;
    pthread_mutex_lock(&the_mutex);
    //pthread_cond_signal(&condp);
    while(count == 0) //wait if nothing to consume
    {
        printf("Consumer Waiting");
        pthread_cond_wait(&condc, &the_mutex);
        //pthread_mutex_lock(&empty);
    }
   // pthread_mutex_unlock(&the_mutex);
    printf("CONSUMER: read buffer \n");

   // pthread_mutex_lock(&the_mutex);
    printf("Last %d, %s \n",last, buffer[0]);
    off = buffer[last];
    last = (last + 1) % BUFSIZE;
    count--;
    pthread_cond_signal(&condp);
    status = execute(off);
    printf("Consumer BACK");
    printf("STATUS %d", status);
    //printf("Consumer got %s \n", retrived);
    pthread_mutex_unlock(&the_mutex);

    pthread_exit(0);
   // pthread_mutex_unlock(&full);
   // pthread_yield();
    //printf("Unlocked");
   // return off;
     //char* res = getFromBuffer();
   // printf("Consumer BACK");
   // status = execute(res);
   // printf("Consumer BACK");
   // printf("STATUS %d", status);

   // pthread_exit(0);
    //printf("STATUS %d", status);

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
   // pthread_create(&c, NULL, consumer, NULL);
    pthread_create(&p, NULL, producer, NULL);
    pthread_create(&c, NULL, consumer, NULL);

    //pthread_join(c, NULL);
    pthread_join(p, NULL);
    pthread_join(c, NULL);

   /* do{
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

    }while(status);*/

    return 0;
}





