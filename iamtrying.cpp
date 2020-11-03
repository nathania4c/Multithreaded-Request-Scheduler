#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

using namespace std;

//master is producer
//slaves are consumers
// read one character of input, then discard up to the newline - do not modify
char readChar() {
  char c = getchar();
  while (getchar() != '\n');
  return c;
}

typedef struct _request_data {
  int id;
  int length;
} request_data;

void Request(int ID, int M){
  int length = (rand() % M) + 1;
  //if full, wait
}

int Master(int N, int M){

  pthread_t slaves[N];
  request_data reqIDs[N];
  int rc;
  
  for (int i = 0; i < N; ++i) {
    sleep(rand()); //sleep for random time 
    Request(i, M); //put request to queue
    if (rc = pthread_create(&slaves[i], NULL, Request(i, M), &reqIDs[i])) { //request()
      fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
      return EXIT_FAILURE;
    }
    sleep(rand()); //sleep for random time
  }
  
  return EXIT_SUCCESS;
  
}

int main(int argc, char **argv) {
  int N; //N slaves threads
  int sleepingTime; //must be random
  int M;
  
  printf("N: ");
  N = readChar() - '0';
  printf("M: ");
  M = readChar() - '0';
  
  Master(N,M);
  
}


int Slave(){
  
  int state; //busy or not
  
  //while request queue empty, wait
  //if there is a request, can only be busy for length amount of time
}
