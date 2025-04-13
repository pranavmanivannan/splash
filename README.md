# splash
`splash` is a simple and performant thread-pool implementation in C++23.

## Core functionality:
- Task submission
	- submit task to a a thread in the thread pool
	- return an int which can be used to index into the thread pool and cancel the thread's execution

- Task queueing
	- submit task to a queue where some available thread will pick up the task 

- Shutdown
	- graceful - allow all threads to finish executing before shutting down
	- forced - shutdown the thread pool immediately without waiting for tasks to finish

- Thread re-use
	- once a thread is instantiated, keep it up for re-use instead of destroying it
    
- Lazy thread instantiation
	- only create a thread when all available threads are already active and the number of instantiated threads < number of configured threads

- Task priority
	- important tasks can be put in the task queue above other tasks
	- use priority queue with default task queue priority set to 0
    
- Thread pinning to cores


Task definition:
- create a task
	- fn, fn params, return value
- submit task and wait for completion


TODO:
- Configurable pool size
	- automatically choose a default size or allow for user to specify # threads
	- "FIXED" - choose some default amount
	- "FIXED", INT num - instantiate `num` threads total
	- "DYNAMIC" - instantiate and destroy threads as needed
- Task cancellation
- Better Task handling