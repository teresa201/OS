/*
@author Teresa Yienger
Throughout their lifetimes.. each Process goes through the New -> Ready -> Running
 ->Blocked(call to sleep) --> Ready -->Running and eventually to the Exit State.
 The processes are interleaved because when one processs is blocked the OS moves the other
 from the ready to running state so it will run while the other is blocked.
Part 2: When the parent process terminates first the PPID of the child changes to init
(which is supposed to be ppid =1 but for some reason my machine makes it 1600).. this is
because when a child process terminates it sends the parent a SIGCHLD signal and waits
for its termination satus to be accepted. Since its parent process terminated and can't
 accept it the kernal ensures that the child process is adopted by "init" which always
accepts its childs termination codes, so that it can terminate.
To run:
Used genuse27
gcc -o sleepy main.c
./sleepy
*/
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

void ChildProcess(int c);
void ParentProcess(int c);

int main(int argc, char *argv[])
{
    //get value of n from command line
    int count;
    if(argc == 2)
        count = atoi(argv[1]);
    else
        count = 5;

    int k, childPid, status;
    int pid = fork();
    if(pid != 0){
       for(k = 1; k <= count; k++)
       {
         int process = getpid();
         int pprocess = getppid();
         sleep(1);
         printf("Original Process with PID %d and PPID: %d - Tick %d \n", process,pprocess, k);
       }
       childPid = wait(&status);
       printf("Child process terminated \n");
    }
    else{
       int j;
       for(j = 1;j <= count; j++)
       {
         int process = getpid();
         int pprocess = getppid();
         sleep(1);
         printf("Child Process with PID %d and PPID: %d - Tick %d \n", process,pprocess, j);
       }
       exit(37);//exit with random number

    }
    printf("Parent Process terminating \n");

}
