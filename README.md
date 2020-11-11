# Multithreaded-Request-Scheduler Java & C++
For Master and N-Slave threads

## How to run
### C++
Compile the c++ file by typing `g++ Part2.cpp -lpthread` then `./a.out`.

The terminal might give you a warning but it has no effect on the program's functionality. 

The program will ask you for the number of slave threads, the maximum duration of each request, and the maximum interval at which the producer waits before making a new requests. Enter an integer for each prompt.

The program will run infinitely. Please press ctrl+c to terminate.

### Design
* The program has a consumer and producer function, and a main function that creates a user inputted number of threads running the consumer function, and a single additional thread to run the producer function. 
* Synchronization is achieved by having 3 mutexes (`buffer_mutex, used_mutex, free_mutex`) and 2 conditional variables (`used_cond, free_cond`). Each conditional variable is linked with its own mutex. The last mutex lock is used when accessing the request queue. 
* When the queue is empty consumers will wait in the conditional variable queue `used_cond` and will be woken by the producer when a request is generated. 
* When the queue is full the producer will wait in the conditional variable queue `free_cond` and will be woken by a consumer when it finishes a request. 

### Java
Compile the java file by typing `javac part1.java`.

Then run the file by typing `java part1`.

The program will ask you for the number of slave threads, the maximum duration of each request, and the maximum interval at which the producer waits before making a new requests. Enter an integer for each prompt.

The program will run infinitely. Please press ctrl+c to terminate.

### Design
* The java program consists of 2 classes, a `main` class and a `Buffer` class. The `main` class includes a main function which is also a driver function that collects user input for the program and creates the consumer threads by creating instances of the `Buffer` class. The main function also directly calls the `producer()` function last.  
* `Buffer` objects created with an integer will be constructed to run the `run()` function which is the consumer function. 
* `Buffer` objects created with 2 integers will be constructed to initialize the class variables.
* Synchronization is achieved with 2 conditional variables `isFull, isEmpty` and a lock `lock` attached to the conditional variables, and the `synchronized` keyword. 
* Consumers sleep in the `isEmpty` queue if the queue is empty, woken by the producer when a request is generated.
* The producer sleeps in the `isFull` queue if the queue is full, woken by a consumer when a request is finished.
* The method `accessBuff()` that is used to access the queue has the `synchronized` keyword which allows for synchronization provided by Java directly. Only 1 consumer or producer can access and modify the queue at any given time. 

## C++ experience
Working with C++ was rather challenging, compared to Java, because the syntax and features/requirements require more attention (ex. due to pointers and such). Thankfully, there are a plethora of online resources and documentations to get help from. However, aside from that, there wasn't much of a difference between working with Java and C++, since most of the challenge was in figuring out the algorithm.

## Individual contribution
All members contributed equally to the project. 
