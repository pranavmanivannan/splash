#pragma once

#include <optional>
#include <thread>
#include <vector>

#include "task.hpp"

namespace splash {

// Core thread pool class. Holds most of the functionality as to how the thread pool should function.
class pool {
  public:
    pool(int num_threads, bool dynamic) : num_threads(num_threads), sig_stop(false) {}
    ~pool();

    // Starts the thread pool
    //
    // Instantiates `num_threads` worker threads which all execute the `run` function.
    // TODO: implement lazy thread instantiation to reduce overhead
    void start() {
      std::lock_guard<std::mutex> lock(thread_pool_m);
      for (int i = 0; i < num_threads; i++) {
        thread_pool.emplace_back(std::thread(&pool::run, this));
      }
    }
    
    // Function that every worker thread runs.
    //
    // Runs an infinite loop. In each iteration, it checks if there is a signal to stop the worker threads.
    // If there is no signal, it waits for a new task to enter the queue, polls it, and then runs the task.
    void run() {
      while (true) {
        if (sig_stop) return;
        // wait for signal
        
        auto t = poll_task();
        if (t.has_value()) {
          // need to modify this after figuring out how to store function in task
          t.value();
        }
      }
    }

    // Submits a task to the thread pool.
    //
    // If the thread pool is of fixed size and full, status will be set to `-2` indicating a failure to submit the task.
    // If the thread pool is dynamic and the task queue is unbounded, it will instead be submitted to a task queue and
    // status will be set to `-1` until it is sent to an active thread.
    void submit_task(task t, int &status);

    // Sends a task to the task queue. 
    //
    // The status will be set to `-1` until an active thread takes the task.
    // If the user defines no priority for a task, it will be treated equal to all other tasks in the queue.
    void queue_task(task t, int &status) {
      if (!t.prio.has_value()) t.prio = 0;

      std::lock_guard<std::mutex> lock(task_queue_m);
      task_queue.emplace(t.prio, t);
      status = -1;
      // send signal to threads that new task exists and they can call poll_task
    }

    // Polls a task from the task queue.
    std::optional<task> poll_task() {
      std::lock_guard<std::mutex> lock(task_queue_m);
      if (task_queue.empty()) {
        return {};
      }
      
      auto task = task_queue.top();
      task_queue.pop();

      return task;
    }

    // Cancels a currently executing task and its thread.
    // void cancel(int thread_num);

    // Shuts down all currently executing threads.
    //
    // If graceful is true, the shutdown allows threads to execute to completion. 
    // Otherwise, all threads are immediately stopped in whatever state they are in.
    void shutdown(bool graceful) {
      std::lock_guard<std::mutex> lock(task_queue_m);
      task_queue = std::priority_queue<task, std::vector<task>>();
    }

    // Delete copy constructor and assignment
    pool(const pool &other) = delete;
    pool& operator=(const pool &other) = delete;

    // Delete move constructor and assignment
    pool(pool &&other) = delete;
    pool& operator=(pool &&other) = delete;

  // private:
    int num_threads;
    bool dynamic;
    bool sig_stop;
    
    std::mutex thread_pool_m;
    std::vector<std::thread> thread_pool;
    
    std::mutex task_queue_m;
    std::priority_queue<task, std::vector<task>> task_queue;
};
} // namespace splash