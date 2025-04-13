# splash
`splash` is a simple and performant thread-pool implementation in C++23.

## Core functionality:
**Task submission**
Submits a task to the task queue.

**Shutdown**

*Graceful*

Allow all threads to finish executing before shutting down. If there are remaining tasks in the task queue, complete the remaining tasks and then shutdown.

*Forced*

Shutdown the thread pool immediately without waiting for tasks to finish.

**Task priority**

Important tasks can be put in the task queue above other tasks by setting task priority at creation time. If not priority is specified, the task queue will assume a default task queue priority of 0.

**Thread pinning**

Threads are pinned to cores when spawned, allowing for `splash` to take advantage of the properties of CPU affinity.

Roadmap:
- Dyanmic thread pool - instantiate and destroy threads as needed
- Task cancellation - requires making task a class and holding status
- Lazy thread instantiation: only create a thread when all available threads are already active and the number of instantiated threads < number of configured threads
