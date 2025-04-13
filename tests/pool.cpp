#include <gtest/gtest.h>
#include "splash.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <numeric>
#include <chrono>
#include <iomanip>

class PoolBasicTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(PoolBasicTest, BasicTaskExecution) {
    // Create a thread pool with 4 threads
    splash::pool<4, false> pool;
    pool.start();

    // Vector to store futures for collecting results
    std::vector<std::future<int>> futures;
    std::vector<std::future<std::string>> string_futures;
    
    // Test 1: Simple arithmetic tasks
    for (int i = 0; i < 10; i++) {
        auto [task, future] = splash::create_task([i]() { 
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return i * i; 
        });
        futures.push_back(std::move(future));
        pool.submit_task(std::move(task), i); // Priority based on index
    }

    // Test 2: String manipulation tasks
    std::vector<std::string> words = {"Hello", "Thread", "Pool", "Testing"};
    for (const auto& word : words) {
        auto [task, future] = splash::create_task([word]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            return word + "!";
        });
        string_futures.push_back(std::move(future));
        pool.submit_task(std::move(task), std::nullopt); // No priority specified
    }

    // Test 3: Fibonacci calculation task with higher priority
    auto [fib_task, fib_future] = splash::create_task([]() {
        auto fib = [](int n) {
            if (n <= 1) return n;
            int a = 0, b = 1;
            for (int i = 2; i <= n; i++) {
                int temp = a + b;
                a = b;
                b = temp;
            }
            return b;
        };
        return fib(20);
    });
    pool.submit_task(std::move(fib_task), 100); // High priority

    // Print results as they complete
    std::cout << "\nSquare results:" << std::endl;
    for (size_t i = 0; i < futures.size(); i++) {
        std::cout << "Task " << i << " result: " << futures[i].get() << std::endl;
    }

    std::cout << "\nString manipulation results:" << std::endl;
    for (size_t i = 0; i < string_futures.size(); i++) {
        std::cout << "String task " << i << " result: " << string_futures[i].get() << std::endl;
    }

    std::cout << "\nFibonacci result: " << fib_future.get() << std::endl;

    // Graceful shutdown
    pool.shutdown(true);
}

TEST_F(PoolBasicTest, ConcurrentMathOperations) {
    splash::pool<4, false> pool;
    pool.start();

    const int num_tasks = 100;
    std::vector<std::future<double>> futures;
    auto start_time = std::chrono::high_resolution_clock::now();

    // Submit multiple math operations
    for (int i = 0; i < num_tasks; i++) {
        auto [task, future] = splash::create_task([i]() {
            // Simulate some complex computation
            double result = 0;
            for (int j = 0; j < 1000; j++) {
                result += std::sin(i * j) * std::cos(j);
            }
            return result;
        });
        futures.push_back(std::move(future));
        pool.submit_task(std::move(task), std::nullopt);
    }

    // Collect and verify results
    std::vector<double> results;
    for (auto& future : futures) {
        results.push_back(future.get());
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "\nProcessed " << num_tasks << " math tasks in " 
              << duration.count() << "ms" << std::endl;
    
    // Verify we got all results
    EXPECT_EQ(results.size(), num_tasks);

    pool.shutdown(true);
}

TEST_F(PoolBasicTest, MixedWorkload) {
    splash::pool<4, false> pool;
    pool.start();

    // Create tasks with different workloads and priorities
    std::vector<std::future<std::string>> results;

    // CPU-intensive task
    auto [cpu_task, cpu_future] = splash::create_task([]() {
        std::string result = "CPU: ";
        long sum = 0;
        for (int i = 0; i < 1000000; i++) {
            sum += i;
        }
        return result + std::to_string(sum);
    });
    pool.submit_task(std::move(cpu_task), 2);
    results.push_back(std::move(cpu_future));

    // I/O-simulation task
    auto [io_task, io_future] = splash::create_task([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return std::string("I/O: Completed");
    });
    pool.submit_task(std::move(io_task), 1);
    results.push_back(std::move(io_future));

    // Memory-intensive task
    auto [mem_task, mem_future] = splash::create_task([]() {
        std::string result = "Memory: ";
        std::vector<int> large_vector(1000000);
        std::iota(large_vector.begin(), large_vector.end(), 0);
        return result + std::to_string(std::accumulate(large_vector.begin(), 
                                                     large_vector.end(), 0L));
    });
    pool.submit_task(std::move(mem_task), 3);
    results.push_back(std::move(mem_future));

    // Print results
    std::cout << "\nMixed workload results:" << std::endl;
    for (auto& future : results) {
        std::cout << future.get() << std::endl;
    }

    pool.shutdown(true);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
