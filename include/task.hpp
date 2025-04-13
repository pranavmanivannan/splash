#pragma once

#include <functional>
#include <future>
#include <memory>

namespace splash
{

// Definition of a task within the thread pool.
using task = std::function<void()>;

template<typename Fn, typename ...Args>
using task_ = std::pair<task, std::future<std::invoke_result_t<Fn, Args...>>>;

/** 
 * @param func      Any function 
 * @param args      Function arguments
 * 
 * Creates a task object which holds a function, task priority, and task status
 */
template<typename Fn, typename ...Args>
task_<Fn, Args...> create_task(Fn&& func, Args&&... args) {
    using ret_type = std::invoke_result_t<Fn, Args...>;

    auto prom = std::make_shared<std::promise<ret_type>>();
    auto fut = prom->get_future();

    auto fn = [_func = std::forward<Fn>(func), _args = std::make_tuple(std::forward<Args>(args)...), prom]() mutable {
        try {
            if constexpr (std::is_void_v<ret_type>) {
                std::apply(_func, _args);
                prom->set_value();
            } else {
                prom->set_value(std::apply(_func, _args));
            }
        } catch (...) {
            prom->set_exception(std::current_exception());
        }
    };
    
    return std::pair{std::move(fn), std::move(fut)};
}

struct task_t {
    int priority;
    task fn;

    bool operator<(const task_t& other) const {
        return priority < other.priority;
    }

    task_t(int p, task t) : priority(p), fn(std::move(t)) {}
};

} // namespace splash
