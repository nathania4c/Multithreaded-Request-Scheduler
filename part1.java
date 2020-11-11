import java.util.*;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.lang.Math.*;
import java.time.*;
import java.util.concurrent.TimeUnit;


public class part1 
{
	public static void main(String args[])
	{
		
		Scanner sc = new Scanner(System.in);
		System.out.println("Enter the number of slave threads: \n");
		int size = sc.nextInt();
		System.out.println("Enter the maximum duration of requests: \n");
		int maxDur = sc.nextInt();
		System.out.println("Enter the interval of request generation: \n");
		int interval = sc.nextInt();	
		Buffer b = new Buffer(maxDur, interval);
		for (int i = 0; i < size; i++)
		{
			new Buffer(i);  //create slaves
		}
		Buffer bb = new Buffer();
		
	}

}


class Buffer implements Runnable
{
	private static Object test;
	private String slaveID;
	private static int[][] buff = new int[10][2];
	private static int duration, maxDur, interval, used, empty, reqID; 
	Thread t;
	//condition variables and lock
	private final ReentrantLock lock = new ReentrantLock();
    private final Condition isFull = lock.newCondition(); 
    private final Condition isEmpty = lock.newCondition();
    
    public Buffer()
    {
    	slaveID = "producer";
    	t = new Thread(this, "producer"); //create slave thread
    	t.start();
    	
    }
	public Buffer(int dur, int val) //initialize variables
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
	
	public Buffer(int id)
	{
		slaveID = Integer.toString(id);
		t = new Thread(this, slaveID); //create slave thread
		System.out.println("Created slave " + slaveID);
		t.start();
	}

	@Override
	public void run() //consumer function
	{
		int[] data = new int[2];
		if (this.slaveID == "producer")
		{
			producer();
		}		
		
		while(true)
		{
			/*lock.lock();
			try
			{
				if (empty == 10)
				{
					System.out.println("slave waiting");
					isEmpty.await(); //queue is empty so sleep until producer makes something
				}
				
			} 
			catch (Exception e) 
			{} 
			finally
			{
				lock.unlock();
			} 
			
			*/
			boolean flag = false;
			try
			{
				TimeUnit.SECONDS.sleep(3);
				flag = checkBuff();
			}
			catch (InterruptedException e)
			{}
			
			if (flag)
			{
				data = accessBuff(0, false);
				System.out.println("Slave " + this.slaveID + " accepted Request " + Integer.toString(data[0]) + " of duration " + Integer.toString(data[1]) + " at time " + LocalTime.now());
			}
			
			try
			{
				TimeUnit.SECONDS.sleep(data[1]);	
			}
			catch (InterruptedException e)
			{}
			
			lock.lock();
			try
			{
				isFull.signal(); //signal to producer that there is free slot
			}
			finally
			{
				lock.unlock();
			}
		}
		

	}
	
	public void producer() 
	{
		int[] test;
		while(true)
		{
			lock.lock();
			try
			{
				if (used == 10)
				{
					isFull.await(); //queue is full so sleep until a slave finishes a request
				}
			} catch (InterruptedException e) 
			{}
			finally
			{
				lock.unlock();
			}

			int range = maxDur;
			duration = (int)(Math.random() * range) + 1;
			test = accessBuff(duration, true);
			System.out.println("Created request " + Integer.toString(reqID) + " of duration " + Integer.toString(duration) + " at time " + LocalTime.now());
			reqID++;
			
			
			lock.lock();
			try
			{
				isEmpty.signal(); //wake one consumer
			}
			catch (Exception e)
			{}
			finally
			{
				lock.unlock();
			}
			
			try
			{
				TimeUnit.SECONDS.sleep(interval); //sleep for interval
			}
			catch (InterruptedException e)
			{}
		}
		
	}
	
	private synchronized int[] accessBuff(int dur, boolean producer) //method to access the queue
	{
		int[] ret = new int[2];
		if (producer) // is producer
		{
			for (int i = 0; i < 10; i++)
			{
				if (buff[i][0] == 0) //if reqID = 0 then there's nothing in it
				{
					--empty;
					++used;
					buff[i][0] = reqID;
					buff[i][1] = dur;
					return ret;
				}
				
			}
		}
		else
		{
			for (int i = 0; i < 10; i++)
			{
				if (buff[i][0] != 0) //if reqID != 0 then there's a request
				{
					--used;
					++empty;
					ret[0] = buff[i][0];
					ret[1] = buff[i][1];
					buff[i][0] = 0; //remove that request from queue
					return ret;
				}
				
			}
		}
		
		return ret;
	}
	
	private synchronized boolean checkBuff()
	{
		for (int i = 0; i < 10; i++)
		{
			if (buff[i][0] != 0) //if reqID = 0 then there's nothing in it
			{
				return true;
			}
			else
			{
				return false;
			}
			
		}
		return false;
	}
	
}
