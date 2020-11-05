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

pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER; //mutex that controls who can edit the request buffer
pthread_mutex_t free_mutex = PTHREAD_MUTEX_INITIALIZER; //mutex used in conjunction with the free_cond conditional variable that consumers can use to wake a producer if the buffer is full
pthread_cond_t  free_cond  = PTHREAD_COND_INITIALIZER; //conditional variable that the producer waits on if the buffer is full, which a consumer signals when they finish a request
pthread_mutex_t used_mutex = PTHREAD_MUTEX_INITIALIZER; //mutex used in conjection with the used_cond conditional variable that the producer uses to wake a consumer when buffer is empty
pthread_cond_t  used_cond  = PTHREAD_COND_INITIALIZER; //conditional variable that the consumers wait on if buffer is empty, which the producer signals when it produces a request

//global variables 
int N; //number of slave threads
int M; //max duration of request
int interval; //interval before producing next request
int buffer [10][2]; //item buffer
int reqID; //request id
int used; //used slots in buffer
int free2; //free slots in buffer, free might be a keyword or something because it does not compile if I use free not free2



void *consumer(void * id)
{
    int *threadID = (int *)id;
    int dur,req;
    while(true)
    {
        pthread_mutex_lock( &used_mutex);
        if (used == 0) //no requests in queue
        {
            pthread_cond_wait( &used_cond, &used_mutex ); //wait for producer to signal if there are no requests
        }
        pthread_mutex_unlock( &used_mutex);
        
        pthread_mutex_lock( &buffer_mutex); //get buffer mutex to read request
        
        for (int i = 0; i < 10; i++) //loop buffer to find a request
        {
            
            if (buffer[i][0] != 0)
            {
                dur = buffer[i][1];
                req = buffer[i][0];
                buffer[i][1] = 0; //remove item from buffer
                buffer[i][0] = 0;
            }
            break;
        }
        
        used = used-1; //update used slots and free slots in buffer
        free2 = free2+1;
        
        pthread_mutex_lock( &free_mutex);
        pthread_cond_signal( &free_cond); //wake producer if it is waiting 
        pthread_mutex_unlock( &free_mutex);
        
	//time stuff
        time_t curr_time;
    	tm * curr_tm;
    	char time_string[100];
    	time(&curr_time);
    	curr_tm = localtime(&curr_time);
    	strftime(time_string, 50, "%T", curr_tm);
        
        printf("Slave %i accepted Request %i of Duration %i at time %s \n", *threadID, req, dur, time_string);
        
        pthread_mutex_unlock( &buffer_mutex);
        
	//consumer sleep
        sleep(dur);
	//time stuff
        time(&curr_time);
    	curr_tm = localtime(&curr_time);
    	strftime(time_string, 50, "%T", curr_tm);
        printf("Slave %i finished Request %i at time %s \n", *threadID, req, time_string);
        
        
    }
}
    
void *producer(void *arg)
{
    int dur;
    
    while(true)
    {
        
        pthread_mutex_lock( &free_mutex);
        if (free2 == 0)
        {
            cout << "buffer full producer waiting \n";
            pthread_cond_wait( &free_cond, &free_mutex ); //if no more free slots wait until the condition variable is signaled
        }
        pthread_mutex_unlock( &free_mutex);
        
        pthread_mutex_lock( &buffer_mutex); //get buffer mutex to write into buffer
        
        for (int i = 0; i < 10; i++) //loop buffer to find a free slot
        {
            if (buffer[i][0] == 0) //if this slot is free
            {
                dur = (rand() % M) + 1;  
                buffer[i][0] = reqID;
                buffer[i][1] = dur;
            }
            break;
        }
        used = used+1; //update used and free slots
        free2 = free2-1;
        
	//time stuff
        time_t curr_time;
    	tm * curr_tm;
    	char time_string[100];
    	time(&curr_time);
    	curr_tm = localtime(&curr_time);
    	strftime(time_string, 50, "%T", curr_tm);
        printf("Created request %i of duration %i at time %s \n", reqID, dur, time_string);
        
        reqID = reqID + 1;
        
        pthread_mutex_lock( &used_mutex);
        pthread_cond_signal( &used_cond); //tell a random consumer to wake up and process
        pthread_mutex_unlock( &used_mutex);
        
        pthread_mutex_unlock( &buffer_mutex);
        
        sleep(interval);
        
    }
}


int main()
{  
    //initialize global variables
    reqID = 1;
    used = 0;
    free2 = 10;

    cout << "Enter number of slave threads \n";
    cin >> N;
    
    cout << "Enter max duration of request \n";
    cin >> M;
  
    cout << "Enter producer interval \n";
    cin >> interval;
    
    N = N+1;
    vector<pthread_t> slaves(N);
    
    for (int i = 0; i < N-1; i++)
    {
        int *x = (int *)i;
        pthread_create(&slaves[i], NULL, consumer, x);
    }
    
    sleep(2); //2 second sleep to get all slaves initialized and waiting before starting producer thread
    pthread_create(&slaves[N], NULL, producer, NULL);
    
    while(true)
    {}
    
    
}
