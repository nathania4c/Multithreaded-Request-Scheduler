import java.util.*;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.lang.Math.*;
import java.time.*;
import java.util.concurrent.TimeUnit;
import java.text.SimpleDateFormat;
import java.util.Date;


public class part1 
{
	//main driver function to get user input and create threads
	public static void main(String args[]) 
	{
		//get user input
		Scanner sc = new Scanner(System.in);
		System.out.print("Enter the number of slave threads: \n");
		int numberOfSlaveThreads = sc.nextInt();
		System.out.print("Enter the maximum duration of requests: \n");
		int maxDuration = sc.nextInt();
		System.out.print("Enter the interval of request generation: \n");
		int maxInterval = sc.nextInt();	
		sc.close();
		
		//initialize variables
		Buffer b = new Buffer(maxDuration, maxInterval); 
		for (int i = 1; i <= numberOfSlaveThreads; i++)
		{
			new Buffer(i);  //create slaves threads
		}
		//run producer thread
		b.producer(); 
		
	}

}

//main threading class with consumer and producer functions
class Buffer implements Runnable 
{
	SimpleDateFormat currentTime = new SimpleDateFormat("HH:mm:ss");
	//private variables
	private String threadID;
	private static int[][] buffer = new int[10][2];//row 0 is reqID and row 1 is duration
	//used is to keep track of the slots in the buffer thats been used, reqID is requestID count
	private static int duration, maxDuration, maxInterval, used, reqID = 1; 
	Thread slaveThread;
	//condition variables and lock
	private static final ReentrantLock lock = new ReentrantLock();
    private static final Condition isFull = lock.newCondition(); 
    private static final Condition isEmpty = lock.newCondition();
    
    //Constructor for producers --- to initialize variables
	public Buffer(int dur, int val) 
	{
		this.maxDuration = dur;
		this.maxInterval = val;
		used = 0;
		//Initialize buffer
		for (int i = 0; i < 10; i++) 
		{
			buffer[i][0] = 0;
		}
	}
	
	//Constructor for consumers --- to run consumer thread
	public Buffer(int id) 
	{
		threadID = Integer.toString(id);
		//create slave thread
		slaveThread = new Thread(this, threadID); 
		System.out.println("Slave Thread ID: " + threadID + " is created");
		slaveThread.start();
	}

	//Consumer method called by thread.start(), accepts request and sleeps
	@Override
	public void run() 
	{
		int[] threadData = new int[2];		
		while(true)
		{
			//lock to check queue status
			lock.lock(); 
			try
			{
				if ((10 - used) == 10) //if there's no request in queue
				{
					System.out.println("Queue is empty, consumer waiting");
					isEmpty.await(); //queue is empty so sleep until producer makes something
					//System.out.println("Slave Thread ID: " + this.threadID +  " is awake");
				}
				
			} 
			catch (Exception e) 
			{} 
			finally
			{
				//done with checking queue
				lock.unlock();
			} 
			
			//Access the queue to get request --- false because consumer
			threadData = accessBufferData(false);
			int duration = threadData[1];
			System.out.println("Slave Thread ID: " + this.threadID + " accepted Request ID: " + Integer.toString(threadData[0]) + " of duration " + Integer.toString(duration) + " at time " + currentTime.format(new Date(System.currentTimeMillis())));
			
			//Process request by sleeping
			try
			{
				TimeUnit.SECONDS.sleep(duration);
				System.out.println("Slave Thread ID: " + this.threadID + " finished Request ID: " + Integer.toString(threadData[0]) + " of duration " + Integer.toString(duration) + " at time " + currentTime.format(new Date(System.currentTimeMillis())));

			}
			catch (InterruptedException e)
			{}
			
			//lock to access isFull condition variable
			lock.lock();
			try
			{
				//Signal to producer that there is free slot in queue
				isFull.signal(); 
			}
			finally
			{
				lock.unlock();
			}
				
		}
	}
	
	//Producer method to generate requests
	public void producer() 
	{
		SimpleDateFormat currentTime = new SimpleDateFormat("HH:mm:ss");
		while(true)
		{
			//Check if queue is full, if full then wait for consumer signal
			lock.lock();
			try
			{
				if (used == 10) //if all slots are full
				{
					System.out.println("Queue is full, producer waiting");
					isFull.await(); //sleep until consumer thread signals that there's a free slot
					//System.out.println("Producer has awaken");
				}
			} catch (InterruptedException e) 
			{}
			finally
			{
				lock.unlock();
			}
			
			//Generate request
			int[] requestData = accessBufferData(true); 
			System.out.println("Request ID: " + Integer.toString(reqID) + " of duration " + Integer.toString(requestData[1]) + " was created at time " + currentTime.format(new Date(System.currentTimeMillis())));
			//update request id count
			reqID++;
			
			//Wake a random consumer to process the request
			lock.lock();
			try
			{
				isEmpty.signal();
			}
			catch (Exception e)
			{}
			finally
			{
				lock.unlock();
			}
			
			//Sleep for random interval seconds before generating another request
			try
			{
				TimeUnit.SECONDS.sleep((int)(Math.random() * maxInterval + 1)); 
			}
			catch (InterruptedException e)
			{}
		}
		
	}
	
	//Method to access the queue, insert request or extract request
	private synchronized int[] accessBufferData(boolean isProducer) 
	{
		int duration = (int)(Math.random() * maxDuration + 1);
		int[] returnValues = new int[2];
		if (isProducer) //If is producer
		{
			for (int i = 0; i < 10; i++)
			{
				if (buffer[i][0] == 0) //if reqID = 0 then there's nothing in it
				{
					++used;
					//insert request in slot
					buffer[i][0] = reqID;
					buffer[i][1] = duration;
					returnValues[0] = 1;
					returnValues[1] = duration;
					return returnValues;
				}
				
			}
		}
		else //If is consumer
		{
			//Loop through queue
			for (int i = 0; i < 10; i++) 
			{
				if (buffer[i][0] != 0) //if reqID != 0 then there's a request in slot
				{
					--used;
					returnValues[0] = buffer[i][0]; //reqID
					returnValues[1] = buffer[i][1]; //duration
					buffer[i][0] = 0; //remove that request from queue
					return returnValues; 
				}
				
			}
		}	
		return returnValues;
	}
	
	
}
