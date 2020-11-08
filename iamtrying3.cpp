#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <ctime>

using namespace std; 

//global variables 
int N; //number of slave threads
int M; //max duration of request
int interval; //interval before producing next request
int reqID; //request id

//shared data between threads
int usedSlots; //used slots in buffer
int freeSlots; //free slots in buffer
int buffer [10][2]; //item buffer with 10 slots

//mutex that controls who can edit the request buffer
pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER; 
//mutex used in conjunction with the free_cond conditional variable that consumers can use to wake a producer if the buffer is full
pthread_mutex_t free_slots_mutex = PTHREAD_MUTEX_INITIALIZER; 
//conditional variable that the producer waits on if the buffer is full, which a consumer signals when they finish a request
pthread_cond_t  free_slots_cond  = PTHREAD_COND_INITIALIZER; 
//mutex used in conjunction with the used_cond conditional variable that the producer uses to wake a consumer when buffer is empty
pthread_mutex_t used_slots_mutex = PTHREAD_MUTEX_INITIALIZER; 
//conditional variable that the consumers wait on if buffer is empty, which the producer signals when it produces a request
pthread_cond_t  used_slots_cond  = PTHREAD_COND_INITIALIZER; 

/* create thread argument struct for thr_func() */
typedef struct _thread_data_t { 
  int slaveID; //slave thread id
} thread_data_t;

void *consumer(void *arg) {
  
  thread_data_t *data = (thread_data_t *)arg; 
  int threadID = (data ->slaveID);
  
  int reqID;
  int duration;
  
  while (true){
    
    //check to see if there are any requests in queue i.e. if any slots are in use
    pthread_mutex_lock( &used_slots_mutex);
    if (usedSlots == 0) //if no requests in queue
    {
      //wait for producer to signal
      pthread_cond_wait( &used_slots_cond, &used_slots_mutex ); 
    }
    pthread_mutex_unlock( &used_slots_mutex);
    
    //get buffer mutex and read request
    pthread_mutex_lock( &buffer_mutex);
    //loop buffer to find a request
    for (int i = 0; i < (*(&buffer + 1) - buffer); i++) 
    {
      if (buffer[i][0] != 0) //if this slot is not empty
      {
        duration = buffer[i][1]; //record the duration
        reqID = buffer[i][0]; //record the request id
        //remove item from buffer
        buffer[i][1] = 0; 
        buffer[i][0] = 0;
      }
      break;
    }
    
    //update used slots and free slots in buffer
    usedSlots -= 1; 
    freeSlots += 1;
    
    //wake the producer if it is waiting for free slots
    pthread_mutex_lock( &free_slots_mutex);
    pthread_cond_signal( &free_slots_cond); 
    pthread_mutex_unlock( &free_slots_mutex);
  }
  
  //time related operatins
  time_t curr_time;
  tm * curr_tm;
  char time_string[100];
  time(&curr_time);
  curr_tm = localtime(&curr_time);
  strftime(time_string, 50, "%T", curr_tm);
  
  printf("Slave %i accepted Request %i of Duration %i at time %s \n", threadID, reqID, duration, time_string);
  
  //release lock on buffer
  pthread_mutex_unlock( &buffer_mutex);
  
  //make the consumer sleep for the set duration
  sleep(duration);
  time(&curr_time);
  curr_tm = localtime(&curr_time);
  strftime(time_string, 50, "%T", curr_tm);
  printf("Slave %i finished Request %i at time %s \n", threadID, reqID, time_string);
  
  //pthread_exit(NULL);
  
}

void *producer(void*arg){
  
  int duration; //duration of request
  
  while(true){
    
    //check if there are any free slots
    pthread_mutex_lock(&free_slots_mutex);
    if (freeSlots == 0) //if no free slots
    {
      cout << "buffer is full, producer thread is waiting \n";
      //wait until the condition variable is signaled i.e. until there is a free slot
      pthread_cond_wait( &free_slots_cond, &free_slots_mutex ); 
    }
    pthread_mutex_unlock( &free_slots_mutex);
    
    //read and write into buffer
    pthread_mutex_lock( &buffer_mutex); 
    //loop buffer to find a free slot
    for (int i = 0; i < (*(&buffer + 1) - buffer); i++) 
    {
      if (buffer[i][0] == 0) //if this slot is free
      {
        duration = (rand() % M) + 1; //generate random duration between 1 - M
        buffer[i][0] = reqID; //store the request id in row 0
        buffer[i][1] = duration; //store the duration in row 1
      }
      break;
    }
    
    //update used and free slots
    usedSlots += 1; 
    freeSlots -= 1;
    
    //Time related operations
    time_t curr_time;
    tm * curr_tm;
    char time_string[100];
    time(&curr_time);
    curr_tm = localtime(&curr_time);
    strftime(time_string, 50, "%T", curr_tm);
    printf("Created request %i of duration %i at time %s \n", reqID, duration, time_string);
    
    //update request id 
    reqID += 1;
    
    //signal a random consumer to wake up and process
    pthread_mutex_lock( &used_slots_mutex);
    pthread_cond_signal( &used_slots_cond); 
    pthread_mutex_unlock( &used_slots_mutex);
    
    //release lock on buffer
    pthread_mutex_unlock( &buffer_mutex);
    
    //sleep before producing the next request
    sleep(interval);
    
  }
}

int main(int argc, char **argv) {
  
  int rc;
  
  //initialize global variables
  reqID = 1;
  
  //initialize shared variables
  usedSlots = 0;
  freeSlots = 10;
  
  //get user input
  cout << "Enter number of slave threads \n";
  cin >> N;
  
  cout << "Enter max duration of request \n";
  cin >> M;
  
  cout << "Enter producer interval \n";
  cin >> interval;
  
  //creating slave threads
  vector<pthread_t> slaves(N+1); 
  thread_data_t thread_data[N];
  for (int i = 0; i < N; i++){
    thread_data[i].slaveID = i+1;
    int *x = &i;
    if ((rc=pthread_create(&slaves[i], NULL, consumer, &thread_data[i]))){
      fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
      return EXIT_FAILURE;
    }
  }
  
  //2 second sleep to get all slaves initialized and waiting before starting producer thread
  sleep(2);
  pthread_create(&slaves[N], NULL, producer, NULL);
  
  while(true)
  {
    printf("hi");
  }
}