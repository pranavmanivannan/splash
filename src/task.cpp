#include <iostream>
#include "task.hpp"

// Example use of create_task

int main() {
    auto [task, fut] = splash::create_task([](int a, int b) { return a + b; }, 2, 3);
    task();
    int result = fut.get();
    std::cout << "Result: " << result << "\n";
    return 0;
}