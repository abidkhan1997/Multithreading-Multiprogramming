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


void DoWorkInChild(int id, char*iFile){

   pthread_t tid1, tid2, tid3;
   thread_data tdata;
   tdata.tid = id;


   pthread_create(&tid1, NULL, sumThread, (void *)&tdata);
   pthread_create(&tid2, NULL, arithmaticThread, (void *)&tdata);
   pthread_create(&tid3, NULL, geometricThread, (void *)&tdata);
   

   pthread_join(tid1, NULL);
   pthread_join(tid2, NULL);
   pthread_join(tid3, NULL);

  
  FILE *fptr;
  fptr = fopen("temp.txt","a");

   if(fptr == NULL)
   {
      printf("Error!");   
      exit(1);             
   }


   fprintf(fptr,"%d %.2f %.2f %.2f\n",id,tdata.geo_avg, tdata.arr_avg,tdata.sum);
 

  fclose(fptr);




   FILE *fptr2 = fopen(iFile,"a");

   if(fptr2 == NULL)
   {
      printf("Error!");   
      exit(1);             
   }



  fprintf(fptr2, "Worker Child Pthread Number = %d : \t Geometric Average = %.2f \t Arithmetic Average = %.2f \t Sum = %.2f \n",id,tdata.geo_avg, tdata.arr_avg,tdata.sum);
  fclose(fptr2);



}


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


readInput(iFile);

 FILE *fptr;
 fptr = fopen("temp.txt","w");
 fclose(fptr);

 fptr = fopen(oFile,"w");
 fclose(fptr);


/* Start children. */
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


fptr = fopen("temp.txt","r");

if(fptr == NULL)
{
  printf("Error!");   
  exit(1);             
}

float sums[SEGMENT];
float a_avg[SEGMENT];
float g_avg[SEGMENT];


for (int i=0; i<SEGMENT; i++)
{
  int id;
  float gs, as, s;
  fscanf(fptr, "%d", &id);
  fscanf(fptr, "%f", &gs);
  fscanf(fptr, "%f", &as);
  fscanf(fptr, "%f", &s);

  sums[id] = s;
  a_avg[id] = as;
  g_avg[id] = gs;


}

fclose(fptr);


 fptr = fopen(oFile,"a");
 
 fprintf(fptr,"Main program thread: Max of the Geometric Averages = %f \n", max_array(g_avg, 16));
 fprintf(fptr,"Main program thread: Max of the Arithmetic Averages = %f \n", max_array(a_avg, 16));
 fprintf(fptr,"Main program thread: Max of the Sums = %f \n", max_array(sums, 16));
 fprintf(fptr,"Main program thread: Terminating. \n");

 fclose(fptr);


}
