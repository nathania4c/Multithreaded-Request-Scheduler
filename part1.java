import java.util.*;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.lang.Math.*;
import java.time.*;
import java.util.concurrent.TimeUnit;


public class part1 
{
	public static void main(String args[]) //main driver function to get user input and create threads
	{
		//get input
		Scanner sc = new Scanner(System.in);
		System.out.println("Enter the number of slave threads: \n");
		int size = sc.nextInt();
		System.out.println("Enter the maximum duration of requests: \n");
		int maxDur = sc.nextInt();
		System.out.println("Enter the interval of request generation: \n");
		int interval = sc.nextInt();	
		Buffer b = new Buffer(maxDur, interval); //initialize variables
		for (int i = 0; i < size; i++)
		{
			new Buffer(i);  //create slaves
		}
		b.producer(); //run producer method
		
	}

}


class Buffer implements Runnable //main threading class with consumer and producer functions
{
	//private variables
	private String slaveID;
	private static int[][] buff = new int[10][2];
	private static int duration, maxDur, interval, used, empty, reqID; 
	Thread t;
	//condition variables and lock
	private static final ReentrantLock lock = new ReentrantLock();
    private static final Condition isFull = lock.newCondition(); 
    private static final Condition isEmpty = lock.newCondition();
    
	public Buffer(int dur, int val) //Producer constructor to initialize variables
	{
		reqID = 1;
		duration = 0;
		maxDur = dur;
		interval = val;
		used = 0;
		empty = 10;
		for (int i = 0; i < 10; i++) //Initialize buffer
		{
			buff[i][0] = 0;
		}
	}
	
	public Buffer(int id) //Consumer constructor to run consumer function
	{
		slaveID = Integer.toString(id);
		t = new Thread(this, slaveID); //create slave thread
		System.out.println("Created slave " + slaveID);
		t.start();
	}

	@Override
	public void run() //Consumer function called by thread.start(), accepts request and sleeps
	{
		int[] data = new int[2];		
		while(true)
		{
			//Atomically check queue status
			lock.lock(); 
			try
			{
				if (empty == 10)
				{
					System.out.println("Queue empty consumer waiting");
					isEmpty.await(); //queue is empty so sleep until producer makes something
					System.out.println("Consumer " + this.slaveID +  " woken");
				}
				
			} 
			catch (Exception e) 
			{} 
			finally
			{
				lock.unlock();
			} 
			
			//Access the queue to get request
			data = accessBuff(0, false);
			System.out.println("Consumer " + this.slaveID + " accepted Request " + Integer.toString(data[0]) + " of duration " + Integer.toString(data[1]) + " at time " + LocalTime.now());
			//Process request by sleeping
			try
			{
				TimeUnit.SECONDS.sleep(data[1]);
				System.out.println("Consumer " + this.slaveID + " finished Request " + Integer.toString(data[0]) + " of duration " + Integer.toString(data[1]) + " at time " + LocalTime.now());

			}
			catch (InterruptedException e)
			{}
			//Signal to producer that there is free slot in queue
			lock.lock();
			try
			{
				isFull.signal(); 
			}
			finally
			{
				lock.unlock();
			}
				
		}
	}
	
	public void producer() //Producer method to generate requests
	{
		while(true)
		{
			//Check if queue is full, if full then wait for consumer signal
			lock.lock();
			try
			{
				if (used == 10)
				{
					System.out.println("Queue full producer waiting");
					isFull.await(); //queue is full so sleep until a slave finishes a request
					System.out.println("Producer woken");
				}
			} catch (InterruptedException e) 
			{}
			finally
			{
				lock.unlock();
			}
			
			//Generate request
			int range = maxDur;
			duration = (int)(Math.random() * range) + 1;
			accessBuff(duration, true); //Insert request to queue
			System.out.println("Created request " + Integer.toString(reqID) + " of duration " + Integer.toString(duration) + " at time " + LocalTime.now());
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
			
			//Sleep for interval seconds before generating another request
			try
			{
				TimeUnit.SECONDS.sleep(interval); 
			}
			catch (InterruptedException e)
			{}
		}
		
	}
	
	private synchronized int[] accessBuff(int dur, boolean producer) //Method to access the queue, insert request or extract request
	{
		
		int[] ret = new int[2];
		if (producer) //If is producer
		{
			for (int i = 0; i < 10; i++)
			{
				if (buff[i][0] == 0) //if reqID = 0 then there's nothing in it
				{
					--empty;
					++used;
					buff[i][0] = reqID;
					buff[i][1] = dur;
					ret[0] = 1;
					return ret;
				}
				
			}
		}
		else //If is consumer
		{
			for (int i = 0; i < 10; i++) //Loop through queue
			{
				if (buff[i][0] != 0) //if reqID != 0 then there's a request
				{
					--used;
					++empty;
					ret[0] = buff[i][0];
					ret[1] = buff[i][1];
					buff[i][0] = 0; //remove that request from queue
					return ret; //return the reqID and duration
				}
				
			}
		}	
		return ret;
	}
	
	
}
