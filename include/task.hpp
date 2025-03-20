#pragma once

#include <functional>

namespace splash
{

// Definition of a task within the thread pool.
class task {
    public:
        task(std::optional<int> priority) : prio(priority) {}
        ~task();
        
        void task_fn();
    // protected:
        std::optional<int> prio;
};

} // namespace splash
