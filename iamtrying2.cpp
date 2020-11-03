#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>

int N; //number of slaves
int M; //1-M random thread length

/* create thread argument struct for thr_func() */
typedef struct _thread_data_t {
  int id;
  int length;
  double stuff;
} thread_data_t;

/* shared data between threads */
double shared_x;
pthread_mutex_t lock_x;

void *thr_func(void *arg) {
  
  thread_data_t *data = (thread_data_t *)arg;
  
  printf("hello from thr_func, thread id: %d\n", data->id);
  /* get mutex before modifying and printing shared_x */
  pthread_mutex_lock(&lock_x);
  shared_x += data->stuff;
  printf("x = %f\n", shared_x);
  pthread_mutex_unlock(&lock_x);
  
  pthread_exit(NULL);
}

char readChar() {
  char c = getchar();
  while (getchar() != '\n');
  return c;
}

int main(int argc, char **argv) {
  
  printf("N: ");
  N = readChar() - '0';
  printf("M: ");
  M = readChar() - '0';
  
  pthread_t slaves[N]; //slave threads
  int i, rc;
  
  /* create a thread_data_t argument array */
  thread_data_t thr_data[N];
  
  /* initialize shared data */
  shared_x = 0;
  
  /* initialize pthread mutex protecting "shared_x" */
  pthread_mutex_init(&lock_x, NULL);
  
  /* create threads */
  while (true){
    sleep(rand()); //sleep for random time 
    thr_data[i].id = i;
    thr_data[i].length = (rand() % M) + 1;
    thr_data[i].stuff = (i + 1) * N;
    if ((rc = pthread_create(&slaves[i], NULL, thr_func, &thr_data[i]))) {
      fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
      return EXIT_FAILURE;
    }
    sleep(rand()); //sleep for random time 
  }
  /* block until all threads complete */
  for (i = 0; i < sizeof(slaves); ++i) {
    pthread_join(slaves[i], NULL);
  }
  
  return EXIT_SUCCESS;
}