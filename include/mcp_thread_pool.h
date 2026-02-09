/**
 * @file mcp_thread_pool.h
 * @brief Simple thread pool implementation
 */

#ifndef MCP_THREAD_POOL_H
#define MCP_THREAD_POOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <type_traits>

namespace mcp {

class thread_pool {
public:
    /**
     * @brief Constructor
     * @param num_threads Number of threads in the thread pool
     */
    explicit thread_pool(unsigned int num_threads = std::thread::hardware_concurrency()) {
        for (unsigned int i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this](std::stop_token stoken) {
                while (!stoken.stop_requested()) {
                    std::function<void()> task;
                    
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex_);
                        condition_.wait(lock, [this, &stoken] {
                            return stoken.stop_requested() || !tasks_.empty();
                        });
                        
                        if (stoken.stop_requested() && tasks_.empty()) {
                            return;
                        }
                        
                        if (!tasks_.empty()) {
                            task = std::move(tasks_.front());
                            tasks_.pop();
                        }
                    }
                    
                    if (task) {
                        task();
                    }
                }
            });
        }
    }
    
    /**
     * @brief Destructor - request stop on all threads and wake them up
     */
    ~thread_pool() {
        auto lock = std::unique_lock(queue_mutex_);
        for (auto& worker : workers_) {
            worker.request_stop();
        }
        condition_.notify_all();
    }
    
    /**
     * @brief Submit task to thread pool
     * @param f Task function
     * @param args Task parameters
     * @return Task future
     */
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type> {
        using return_type = typename std::invoke_result<F, Args...>::type;
        
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        std::future<return_type> result = task->get_future();
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            tasks_.emplace([task]() { (*task)(); });
        }
        
        condition_.notify_one();
        return result;
    }
    
private:
    // Worker threads (jthread automatically joins on destruction)
    std::vector<std::jthread> workers_;

    // Task queue
    std::queue<std::function<void()>> tasks_;
    
    // Mutex and condition variable
    std::mutex queue_mutex_;
    std::condition_variable condition_;
};

} // namespace mcp

#endif // MCP_THREAD_POOL_H 