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
int maxInterval;
int interval; //interval before producing next request
int reqCount; //sequentially increasing request id

//shared data between threads
int usedSlots; //used slots in buffer
int buffer [10][2] = {0}; //item buffer with 10 slots
int busyThread = 0;

//mutex that controls who can edit the request buffer
pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t running_mutex = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t  running_cond  = PTHREAD_COND_INITIALIZER; 
//mutex used in conjunction with the free_cond conditional variable that consumers can use to wake a producer if the buffer is full
pthread_mutex_t free_slots_mutex = PTHREAD_MUTEX_INITIALIZER; 
//conditional variable that the producer waits on if the buffer is full, which a consumer signals when they finish a request
pthread_cond_t  free_slots_cond  = PTHREAD_COND_INITIALIZER; 
//mutex used in conjunction with the used_cond conditional variable that the producer uses to wake a consumer when buffer is empty
pthread_mutex_t used_slots_mutex = PTHREAD_MUTEX_INITIALIZER; 
//conditional variable that the consumers wait on if buffer is empty, which the producer signals when it produces a request
pthread_cond_t  used_slots_cond  = PTHREAD_COND_INITIALIZER; 

typedef struct _thread_data_t{
  int id;
  int reqID;
  int duration;
} thread_data_t;

void *consumer(void *arg) {
  
  thread_data_t *data = (thread_data_t*)arg;
  int threadID = (data -> id);
  
  thread_data_t running_thread_data;
  
  int reqID;
  int duration;
  
  while (true){
    
    //check to see if there are any requests in queue i.e. if any slots are in use
    pthread_mutex_lock( &used_slots_mutex);
    if (usedSlots == 0) //if no requests in queue
    {
      //wait for producer to signal
      cout << "nothing in the waiting queue, consumer thread is waiting \n";
      pthread_cond_wait( &used_slots_cond, &used_slots_mutex ); 
    }
    pthread_mutex_unlock( &used_slots_mutex);
    
    //get buffer mutex and read request
    pthread_mutex_lock( &buffer_mutex);
    //loop buffer to find a request
    for (int i = 0; i < 10; i++) 
    {
      if (buffer[i][0] != 0) //if there's no threads waiting
      {
            duration = buffer[i][1]; 
            reqID = buffer[i][0];
            //remove item from queue
            buffer[i][1] = 0; 
            buffer[i][0] = 0;
            
            //update used slots and free slots in buffer
            pthread_mutex_lock(&used_slots_mutex);
            usedSlots = usedSlots - 1;
            pthread_mutex_unlock(&used_slots_mutex);
            
            //wake the producer if it is waiting for free slots
            pthread_mutex_lock( &free_slots_mutex);
            pthread_cond_signal( &free_slots_cond); 
            pthread_mutex_unlock( &free_slots_mutex);
        }
      break;
    }
    pthread_mutex_unlock( &buffer_mutex);
    
      pthread_mutex_lock( &running_mutex);
      if (busyThread != 0){
        pthread_cond_wait( &running_cond, &running_mutex ); 
      }
      pthread_mutex_unlock( &running_mutex);
        busyThread = threadID;
      
      //time related operations
      time_t curr_time;
      tm * curr_tm;
      char time_string[100];
      time(&curr_time);
      curr_tm = localtime(&curr_time);
      strftime(time_string, 50, "%T", curr_tm);
      
      printf("Slave %i accepted Request %i of Duration %i at time %s \n", threadID, reqID, duration, time_string);
      
      //making the consumer busy for set duration by sleeping
      sleep(duration);
      time(&curr_time);
      curr_tm = localtime(&curr_time);
      strftime(time_string, 50, "%T", curr_tm);
      printf("Slave %i finished Request %i at time %s \n", threadID, reqID, time_string);
      
      busyThread=0;
      pthread_mutex_lock( &running_mutex);
      pthread_cond_signal( &running_cond); 
      pthread_mutex_unlock( &running_mutex);
  }
}

void *producer(void*arg){
  
  int duration; //duration of request
  
  while(true){
    
    //check if there are any free slots
    pthread_mutex_lock(&free_slots_mutex);
    if (10-usedSlots == 0) //if no free slots
    {
      cout << "buffer is full, producer thread is waiting \n";
      //wait until the condition variable is signaled i.e. until there is a free slot
      pthread_cond_wait( &free_slots_cond, &free_slots_mutex ); 
    }
    pthread_mutex_unlock( &free_slots_mutex);
    
    //read and write into buffer
    pthread_mutex_lock( &buffer_mutex); 
    //loop buffer to find a free slot
    for (int i = 0; i < 10; i++) 
    {
      if (buffer[i][0] == 0) //if this slot is free
      {
        duration = (rand() % M) + 1; //generate random duration between 1 - M
        buffer[i][0] = reqCount; //store the request id in row 0
        buffer[i][1] = duration; //store the duration in row 
      }
      break;
    }
    //release lock on buffer
    pthread_mutex_unlock( &buffer_mutex);
    
    //update used and free slots
    usedSlots = usedSlots+1; 
    
    //Time related operations
    time_t curr_time;
    tm * curr_tm;
    char time_string[100];
    time(&curr_time);
    curr_tm = localtime(&curr_time);
    strftime(time_string, 50, "%T", curr_tm);
    printf("Created request %i of duration %i at time %s \n", reqCount, duration, time_string);
    
    //update request id 
    reqCount = reqCount + 1;
    
    //signal a random consumer to wake up and process
    pthread_mutex_lock( &used_slots_mutex);
    pthread_cond_signal( &used_slots_cond); 
    pthread_mutex_unlock( &used_slots_mutex);
    
    //sleep for a random interval before producing the next request
    interval = (rand() % maxInterval);
    sleep(interval);
 
  }
}

int main(int argc, char **argv) {
  
  int rc;
  
  //initialize global variables
  reqCount = 1;
  
  //initialize shared variables
  usedSlots = 0;
  
  //get user input
  cout << "Enter number of slave threads \n";
  cin >> N;
  
  cout << "Enter max duration of request \n";
  cin >> M;
  
  cout << "Enter producer interval \n";
  cin >> maxInterval;
  
  //creating slave threads
  N = N+1;
  vector<pthread_t> slaves(N);
  
  for (int i = 0; i < N-1; i++)
  {
    int *x = &i;
    if ((rc=pthread_create(&slaves[i], NULL, consumer, x))){
      fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
      return EXIT_FAILURE;
    }
  }
  
  //2 second sleep to get all slaves initialized and waiting before starting producer thread
  sleep(2);
  pthread_create(&slaves[N], NULL, producer, NULL);
  
  while(true)
  {}
}