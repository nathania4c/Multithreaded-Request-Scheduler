#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int N; //number of slaves
int M; //1 to M random thread length
int S; //size of buffer

/* shared data between threads */
int shared_x; //will be recording the length of the threads
pthread_mutex_t lock_x; //semaphore lock

/* create thread argument struct for thr_func() */
typedef struct _thread_data_t { 
  int id; //slave thread id
  int length; //this will be randomly generated
} thread_data_t;

void *slave(void *arg) {
  
  thread_data_t *data = (thread_data_t *)arg; //get arguments passed
  
  sleep(data->length); //sleep for length amount of time?
  
  printf("running from slave thread id = %d\n", data->id);
  /* get mutex before modifying and printing shared_x */
  pthread_mutex_lock(&lock_x);
  //enter critical section
  shared_x = data->length; 
  printf("this request ran for %d time units\n", shared_x);
  //exit critical section
  pthread_mutex_unlock(&lock_x);
  
  pthread_exit(NULL);
}

/*got this from previous lab */
char readChar() {
  char c = getchar();
  while (getchar() != '\n');
  return c;
}

int main(int argc, char **argv) {
  
  //getting user input
  printf("Number of slaves N: ");
  N = readChar() - '0'; //getting the number of slaves to generate
  printf("Random length 1 to M: ");
  M = readChar() - '0'; //getting the M to generate random length
  printf("Size of the buffer S: ");
  S = readChar() - '0'; //getting the S for the size of the bounded buffer
  
  pthread_t slaves[N]; //array of slave threads
  int i, rc;
  
  /* create a thread_data_t argument array */
  thread_data_t thread_data[S];
  //int queue[S];
  
  /* initialize shared data */
  shared_x = 0; 
  
  /* initialize pthread mutex protecting "shared_x" */
  pthread_mutex_init(&lock_x, NULL);
  
  printf("running from the master thread\n");
  
  /* create threads */
  while (i < N){
    sleep(rand()%10); //sleep for random time before making a request
    thread_data[i].id = i+1; //slave id starts with 1 to N
    thread_data[i].length = (rand() % M) + 1; //randomly generated length of request
    if ((rc = pthread_create(&slaves[i], NULL, slave, &thread_data[i]))) {
      fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
      return EXIT_FAILURE;
    }
    i++;
    sleep(rand()%10); //sleep for random time before doing another loop?
  }
  /* block until all threads complete
  for (i = 0; i < Ns; ++i) {
    pthread_join(slaves[i], NULL);
  }
  printf("%lu",*(&thread_data + 1) - thread_data);
   */
  
  printf("bye\n");
  
  return EXIT_SUCCESS;
}