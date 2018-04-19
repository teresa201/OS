//Teresa Yienger
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define CLOCK_SIZE 4



void writeClockToFile(FILE* filePtr, int p[], int u[], int m[], int n) {
      int i = 0;
      fprintf(filePtr, " FRAME       PAGE       USE        MODIFY\n");
      for (  ; i < CLOCK_SIZE; i++)
      {
        if(n != i)
        {
            fprintf(filePtr, " % d          % d          % d          % d\n",i, p[i],u[i],m[i]);
        }
        else
        {
            fprintf(filePtr, " % d          % d          % d          % d  <- Next Frame\n",i, p[i],u[i],m[i]);
        }
      }
      fflush(filePtr);

};

int moveNext(int next, int index){
    if(index == CLOCK_SIZE -1){
        return 0;
    }
    else{
        next = index + 1;
        return next;
    }
}



int main()
{
      setbuf(stdout, NULL);
      char inFileName[ ] = "testdata.txt";
      FILE *inFilePtr = fopen(inFileName, "r");
      if(inFilePtr == NULL) {
            printf("File %s could not be opened.\n", inFileName);
            exit(1);
      }

      char outFileName[ ] = "results.txt";
      FILE *outFilePtr = fopen(outFileName, "w");
      if(outFilePtr == NULL) {
            printf("File %s could not be opened.\n", outFileName);
            exit(1);
      }

      int page;
      char operation;
      fscanf(inFilePtr, "%d%c", &page, &operation);
      int pagec[CLOCK_SIZE] = {[0 ... (CLOCK_SIZE - 1)] = -1};
      int useBit[CLOCK_SIZE] = {0};
      int modifyBit[CLOCK_SIZE] = {0};
      int nxt = 0;

      while(!feof(inFilePtr)){
            fprintf(outFilePtr, "Page referenced: %d %c\n", page, operation);
            bool found = false;
            int k;
            int fIndex;
            //check to see if page is in clock
            for(k = 0; k < CLOCK_SIZE; k++){
                if(page  == pagec[k]){
                    found = true;
                    fIndex = k;
                    break;
                }
            }
            //if page is in clock, update flags
            if(found){
                //change modify bit if it was 0
                if (modifyBit[k] == 0)
                {
                    modifyBit[k] = 1;
                }
                //write to file
                writeClockToFile(outFilePtr, pagec, useBit, modifyBit, nxt);
            }
            else
            {
                bool putIn = false;
                int round = 1;
                int k;
                while(!putIn)
                {
                    if((round <= CLOCK_SIZE) || ( (round >= 2* CLOCK_SIZE + 1)
                                                 && (round <= 3 * CLOCK_SIZE)))
                    {//Step 3
                        for(k = 0; k < CLOCK_SIZE; k++)
                        {
                            int index = (k + nxt) % CLOCK_SIZE;
                            int use = useBit[index];
                            int modify = modifyBit[index];
                            if((use == 0) && (modify == 0))
                            {
                                //replace page
                                pagec[index] = page;
                                //set use bit
                                useBit[index] = 1;
                                //set modify bit
                                if(operation == 'r'){
                                   modifyBit[index] = 0;
                                }
                                else{
                                   modifyBit[index] = 1;
                                }
                                nxt = moveNext(nxt, index);
                                putIn = true;
                                break;
                            }
                            else
                            {
                                round++;
                            }

                        }
                    }
                    else if(((round > CLOCK_SIZE) && (round <= 2* CLOCK_SIZE)) ||
                            (round > 3 * CLOCK_SIZE ))
                    { //Step 4
                        for(k = 0; k < CLOCK_SIZE; k++)
                        {
                            int index = (k + nxt) % CLOCK_SIZE;
                            int use = useBit[index];
                            int modify = modifyBit[index];
                            if((use == 0) && (modify == 1))
                            {
                                //replace page
                                pagec[index] = page;
                                //set use bit
                                useBit[index] = 1;
                                //set modify bit
                                if(operation == 'r'){
                                   modifyBit[index] = 0;
                                }
                                else{
                                   modifyBit[index] = 1;
                                }
                                nxt = moveNext(nxt, index);
                                putIn = true;
                                break;
                            }
                            else
                            {
                                useBit[index] = 0;
                                round++;
                            }
                        }
                    }

                }
                writeClockToFile(outFilePtr, pagec, useBit, modifyBit, nxt);
            }



            //writeClockToFile(outFilePtr);
            fscanf(inFilePtr, "%d%c", &page, &operation);

      }//end while



      fclose(inFilePtr);
      fclose(outFilePtr);

      return 0;

};
