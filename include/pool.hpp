/*
 * Core functionality of the `splash` library. This file contains the implementation
 * of the `pool` class and its methods. All functionality of `pool` is defined here.
 */

#pragma once

#include <optional>
#include <thread>
#include <queue>
#include <vector>
#include <pthread.h>

#include "sys.hpp"
#include "task.hpp"

namespace splash {

template<uint num_threads, bool dynamic>
class pool {
  public:
    pool() : sig_stop(false) {}
    ~pool() {
      std::lock_guard<std::mutex> lk(thread_pool_m);
      if (thread_pool.empty()) return;
      shutdown(true);
    }

    /**
     * Starts the thread pool.
     *
     * Instantiates `num_threads` worker threads which all execute the `run` function.
     * TODO: implement lazy thread instantiation to reduce overhead
     */
    void start() {
      uint core_id = 0;

      std::lock_guard<std::mutex> lock(thread_pool_m);
      for (uint i = 0; i < num_threads; i++) {
        core_id = i % N_THREADS;

        // figure out how to pass in core id
        thread_pool.emplace_back(std::thread(&pool::run, this, core_id));
      }
    }

    /**
     * Function that every worker thread runs.
     *
     * Runs an infinite loop. In each iteration, it checks if there is a signal to stop the worker threads.
     * If there is no signal, it waits for a new task to enter the queue, polls it, and then runs the task.
     */
    void run(int core_id) {
      // Pins thread to core on non-macOS systems and suggests higher QoS on macOS.
      if (SYSTEM_T == 0) {
        set_qos_affinity(3);
      } else if (SYSTEM_T == 1 || SYSTEM_T == 2) {
        pin_thread_to_core(core_id);
      }

      while (true) {
        std::unique_lock lk(task_queue_m);
        task_queue_cv.wait(lk, [this]() {
          return sig_stop || !task_queue.empty();
        });

        if (sig_stop && task_queue.empty())
          return;

        auto t = poll_task();
        lk.unlock();

        if (t.has_value()) {
          auto task_t = t.value();
          task_t.fn();
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
      task_queue.emplace(task_t(priority.value_or(0), std::move(t)));
      lk.unlock();
      task_queue_cv.notify_all();
    }

    // Polls a task from the task queue. Expects caller to hold `task_queue_m` when calling.
    std::optional<task_t> poll_task() {
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
      std::lock_guard<std::mutex> thread_pool_l(thread_pool_m);
      if (thread_pool.empty()) return;

      {
        std::lock_guard<std::mutex> lk(task_queue_m);
        sig_stop = true;
      }

      task_queue_cv.notify_all();
      
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

      thread_pool.clear();
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
    std::priority_queue<task_t, std::vector<task_t>> task_queue;
    std::condition_variable task_queue_cv;
};
} // namespace splash