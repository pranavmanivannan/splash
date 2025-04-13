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
template<uint num_threads, bool dynamic>
class pool {
  public:
    pool() : sig_stop(false) {}
    ~pool();

    /**
     * Starts the thread pool.
     *
     * Instantiates `num_threads` worker threads which all execute the `run` function.
     * TODO: implement lazy thread instantiation to reduce overhead
     */
    void start() {
      int core_id = 0;

      std::lock_guard<std::mutex> lock(thread_pool_m);
      for (int i = 0; i < num_threads; i++) {
        core_id = i % N_THREADS;

        // figure out how to pass in core id
        thread_pool.emplace_back(std::thread(&pool::run, this));
      }
    }

    /**
     * Function that every worker thread runs.
     *
     * Runs an infinite loop. In each iteration, it checks if there is a signal to stop the worker threads.
     * If there is no signal, it waits for a new task to enter the queue, polls it, and then runs the task.
     */
    void run(int core_id) {
      if (SYSTEM_T == 0) {
        set_qos_affinity(3);
      } else if (SYSTEM_T == 1 || SYSTEM_T == 2) {
        pin_thread_to_core(core_id);
      }

      while (true) {
        std::unique_lock lk(task_queue_m);
        if (sig_stop && task_queue.empty()) return;
        task_queue_cv.wait(lk);
        
        auto t = poll_task();
        lk.unlock();

        if (t.has_value()) {
          // TODO: need to modify this after figuring out how to store function in task
          auto fn = t.value();
        }
      }
    }


    /**
     * @param t Task for submission.
     * 
     * Submits a task to the task queue. If no priority is defined for the task, 
     * it will be treated equal to all other tasks in the queue.
     */
    void submit_task(task t, std::optional<int> priority) {
      std::unique_lock lk(task_queue_m);
      task_queue.emplace(priority.value_or(0), t);
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

    /**
     * Shuts down all currently executing threads.
     *
     * If graceful is true, the shutdown allows threads to execute to completion.
     * Otherwise, all threads are immediately stopped in whatever state they are in.
     */
    void shutdown(bool graceful) {
      sig_stop = true;

      std::lock_guard<std::mutex> thread_pool_l(thread_pool_m);
      if (graceful) {
        for (auto &t : thread_pool) {
          t.join();
        }
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

  private:
    bool sig_stop;
    
    std::mutex thread_pool_m;
    std::vector<std::thread> thread_pool;
    
    std::mutex task_queue_m;
    std::priority_queue<task, std::vector<task>> task_queue;
    std::condition_variable task_queue_cv;
};
} // namespace splash