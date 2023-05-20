#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SEGMENT 16

int numberArray[16000];

   
pid_t pids[16];
int i;
int n = 16;

typedef struct thread_data {
//    int input[1000];
   int tid;
   float sum;
   float geo_avg;
   float arr_avg;


} thread_data;

//this thread calculate summations for each segment

void *sumThread(void *arg)
{
   thread_data *tdata=(thread_data *)arg;

    float s = 0;
    for (i = 0; i < 1000; i++){
        s = s + numberArray[1000*tdata->tid + i];
    }


   tdata->sum=s;
   
   pthread_exit(NULL);
}


//this thread calculate geometric average for each segment

void *geometricThread(void *arg)
{
   thread_data *tdata=(thread_data *)arg;

    float s = 0;
    for (i = 0; i < 1000; i++){
        s = s + log ( numberArray[1000*tdata->tid + i]);
    }


   s = s / 1000.0;
   tdata->geo_avg=  exp(s);

   pthread_exit(NULL);
}


//this thread calculate arithmatic average for each segment

void *arithmaticThread(void *arg)
{
   thread_data *tdata=(thread_data *)arg;

    float s = 0;
    for (i = 0; i < 1000; i++){
        s = s + numberArray[1000*tdata->tid + i];
    }


   tdata->arr_avg= s /1000.0;


   pthread_exit(NULL);
}


/*

this is a child process.
it will create 3 seperate thead to do the caculations like sum, geometric average, arithmatic average.

*/

void DoWorkInChild(int id, char*iFile){

   pthread_t tid1, tid2, tid3;
   thread_data tdata;
   tdata.tid = id;


   //creating 3 seperate threads
   pthread_create(&tid1, NULL, sumThread, (void *)&tdata);
   pthread_create(&tid2, NULL, arithmaticThread, (void *)&tdata);
   pthread_create(&tid3, NULL, geometricThread, (void *)&tdata);
   

   //waiting for all the threads to finish
   pthread_join(tid1, NULL);
   pthread_join(tid2, NULL);
   pthread_join(tid3, NULL);

  
  //save result to temporary file
  FILE *fptr;
  fptr = fopen("temp.txt","a");

   if(fptr == NULL)
   {
      printf("Error!");   
      exit(1);             
   }


   fprintf(fptr,"%d %.2f %.2f %.2f\n",id,tdata.geo_avg, tdata.arr_avg,tdata.sum);
 
  fclose(fptr);




	
   //saving result to the final output file.
   FILE *fptr2 = fopen(iFile,"a");

   if(fptr2 == NULL)
   {
      printf("Error!");   
      exit(1);             
   }

  fprintf(fptr2, "Worker Child Pthread Number = %d : \t Geometric Average = %.2f \t Arithmetic Average = %.2f \t Sum = %.2f \n",id,tdata.geo_avg, tdata.arr_avg,tdata.sum);
  fclose(fptr2);



}


/*
this function read the user given input file.
Save all the data in numberArray.
Number array is used by all the threads to do all the calculations like sum, geometric average, arithmatic average.

*/


void readInput(char*filename)
{
    FILE *myFile;
    myFile = fopen(filename, "r");

    //read file into array
    int i;

    if (myFile == NULL){
        printf("Error Reading File\n");
        exit (0);
    }

    float s = 0;
    for (i = 0; i < 16000; i++){
        fscanf(myFile, "%d,", &numberArray[i] );
        s = s + numberArray[i];
    }

}


// find maximum from an array

float max_array(float a[], int num_elements)
{
   float max=-1;
   for (int i=0; i<num_elements; i++)
   {


    if (a[i]>max)
    {
        max=a[i];
    }
   }
   return(max);
}



int main(int argc, char *argv[])
{

char * iFile = argv[1];
char * oFile = argv[2];

// printf("%s %s", iFile, oFile);


//read userinput
readInput(iFile);


// create a temporary file to store intenal results from all the threads
 FILE *fptr;
 fptr = fopen("temp.txt","w");
 fclose(fptr);


// create outfile file for the final result
 fptr = fopen(oFile,"w");
 fclose(fptr);


/* Start 16 child process. */
for (i = 0; i < n; ++i) {
  if ((pids[i] = fork()) < 0) {
    perror("fork");
    abort();
  } else if (pids[i] == 0) {
    DoWorkInChild(i, oFile);
    exit(0);
  }
}

/* Wait for children to exit. */
int status;
pid_t pid;
while (n > 0) {
  pid = wait(&status);
//   printf("Child with PID %ld exited with status 0x%x.\n", (long)pid, status);
  --n;  // TODO(pts): Remove pid from the pids array.
}


// at this point all the calculations are done.
// temporary results are in temp.txt file
// now opening the file to output from temp.txt file to the final output file
fptr = fopen("temp.txt","r");

if(fptr == NULL)
{
  printf("Error!");   
  exit(1);             
}

float sums[SEGMENT];
float a_avg[SEGMENT];
float g_avg[SEGMENT];


//reading all results from the temp.txt file
for (int i=0; i<SEGMENT; i++)
{
  int id;
  float gs, as, s;
  fscanf(fptr, "%d", &id);
  fscanf(fptr, "%f", &gs);
  fscanf(fptr, "%f", &as);
  fscanf(fptr, "%f", &s);

  //storing temporary result in the array to find maximum of all segments
  sums[id] = s;
  a_avg[id] = as;
  g_avg[id] = gs;


}

fclose(fptr);


//opening the output file to write final result
 fptr = fopen(oFile,"a");
 
 //writting all results in output file
 fprintf(fptr,"Main program thread: Max of the Geometric Averages = %f \n", max_array(g_avg, 16));
 fprintf(fptr,"Main program thread: Max of the Arithmetic Averages = %f \n", max_array(a_avg, 16));
 fprintf(fptr,"Main program thread: Max of the Sums = %f \n", max_array(sums, 16));
 fprintf(fptr,"Main program thread: Terminating. \n");

 fclose(fptr);


}
