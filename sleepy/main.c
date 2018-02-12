#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
/*
@author Teresa Yienger
The process id is the same within a run but different each time you run the program
After the sleep function the process transitions to the ready state
*/
int main(int argc, char *argv[])
{
    int count;
    if(argc == 2)
        count = atoi(argv[1]);
    else
        count = 5;
    int k;
    int process1 = getpid();
     printf("Process Id: %d - Start \n", process1);
    for(k = 1; k <= count; k++)
    {
      int process = getpid();
      sleep(1);
      printf("Process Id: %d - Tick %d \n", process, k);
    }
}

