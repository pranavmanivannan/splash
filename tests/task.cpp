#include <gtest/gtest.h>
#include "task.hpp"
#include <string>
#include <stdexcept>
#include <thread>
#include <chrono>

// Test suite for splash::create_task
class TaskTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Test creating and executing a task with a simple lambda that returns an int
TEST_F(TaskTest, SimpleIntegerTask) {
    auto [task, future] = splash::create_task([](int a, int b) { return a + b; }, 2, 3);
    task();
    EXPECT_EQ(future.get(), 5);
}

// Test with a void return type
TEST_F(TaskTest, VoidReturnTask) {
    bool flag = false;
    auto [task, future] = splash::create_task([&flag]() { flag = true; });
    task();
    future.get(); // Wait for completion
    EXPECT_TRUE(flag);
}

// Test with a string return type
TEST_F(TaskTest, StringTask) {
    auto [task, future] = splash::create_task(
        [](const std::string& a, const std::string& b) { return a + b; },
        "Hello, ", "World!"
    );
    task();
    EXPECT_EQ(future.get(), "Hello, World!");
}

// Test exception handling
TEST_F(TaskTest, ExceptionHandling) {
    auto [task, future] = splash::create_task([]() { throw std::runtime_error("Test error"); });
    task();
    EXPECT_THROW(future.get(), std::runtime_error);
}

// Test with a complex callable object (functor)
class Multiplier {
public:
    int operator()(int x, int y) const { return x * y; }
};

TEST_F(TaskTest, FunctorTask) {
    Multiplier mult;
    auto [task, future] = splash::create_task(mult, 4, 5);
    task();
    EXPECT_EQ(future.get(), 20);
}

// Test with a member function
class TestClass {
public:
    int add(int a, int b) { return a + b; }
};

TEST_F(TaskTest, MemberFunctionTask) {
    TestClass obj;
    auto [task, future] = splash::create_task(&TestClass::add, &obj, 3, 4);
    task();
    EXPECT_EQ(future.get(), 7);
}

// Test with async execution
TEST_F(TaskTest, AsyncExecution) {
    auto start_time = std::chrono::steady_clock::now();
    
    auto [task, future] = splash::create_task([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return 42;
    });
    
    std::thread t(std::move(task));
    auto result = future.get();
    t.join();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start_time
    ).count();
    
    EXPECT_EQ(result, 42);
    EXPECT_GE(duration, 100);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}