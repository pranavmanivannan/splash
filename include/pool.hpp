/*
 * Core functionality of the `splash` library. This file contains the implementation
 * of the `pool` class and its methods. All functionality of `pool` is defined here.
 */

#pragma once

#include <optional>
#include <thread>
#include <vector>
#include <pthread.h>

#include "sys.hpp"
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
      int core_id = 0;

      std::lock_guard<std::mutex> lock(thread_pool_m);
      for (int i = 0; i < num_threads; i++) {
        core_id = i % N_THREADS;
        thread_pool.emplace_back(std::thread(&pool::run, this));
      }
    }
    
    // Function that every worker thread runs.
    //
    // Runs an infinite loop. In each iteration, it checks if there is a signal to stop the worker threads.
    // If there is no signal, it waits for a new task to enter the queue, polls it, and then runs the task.
    void run(int core_id) {
      if (SYSTEM_T == 0) {
        set_qos_affinity(3);
      } else if (SYSTEM_T == 1 || SYSTEM_T == 2) {
        pin_thread_to_core(core_id);
      }

      while (true) {
        if (sig_stop) return;

        // wait for signal and poll task queue
        std::unique_lock lk(task_queue_m);
        task_queue_cv.wait(lk);
        
        auto t = poll_task();
        lk.unlock();

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
    void submit_task(task t, int &status) {
      // TODO: change queueing the task to actually putting the task on the thread pool
      queue_task(t, status);
    }

    // Sends a task to the task queue. 
    //
    // The status will be set to `-1` until an active thread takes the task.
    // If the user defines no priority for a task, it will be treated equal to all other tasks in the queue.
    void queue_task(task t, int &status) {
      if (!t.prio.has_value()) t.prio = 0;

      std::unique_lock lk(task_queue_m);
      task_queue.emplace(t.prio, t);
      status = -1;

      // Send signal to threads that a new task exists and they can call poll_task
      lk.unlock();
      task_queue_cv.notify_one(); // TODO: might be notify_all
    }

    // Polls a task from the task queue. Expects caller to hold `task_queue_m` when calling.
    std::optional<task> poll_task() {
      if (task_queue.empty()) {
        return {};
      }
      
      auto task = task_queue.top();
      task_queue.pop();

      return task;
    }

    // Cancels a currently executing task and its thread.
    // TODO: void cancel(int thread_num);

    // Shuts down all currently executing threads.
    //
    // If graceful is true, the shutdown allows threads to execute to completion. 
    // Otherwise, all threads are immediately stopped in whatever state they are in.
    void shutdown(bool graceful) {
      std::lock_guard<std::mutex> task_queue_l(task_queue_m);
      task_queue = std::priority_queue<task, std::vector<task>>(); 

      std::lock_guard<std::mutex> thread_pool_l(thread_pool_m);
      if (graceful) {

      } else {
        for (auto &t : thread_pool) {
          pthread_t handle = t.native_handle();
          pthread_cancel(handle);
          t.join();
        }
      }
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
    std::condition_variable task_queue_cv;
};
} // namespace splash