#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <iostream>
#include <iomanip>

namespace msgsdk {

// 获取日志互斥锁的辅助函数（使用函数内静态变量，确保线程安全且唯一）
inline std::mutex& get_log_mutex() {
    static std::mutex log_mutex;
    return log_mutex;
}

template<typename T>
class BlockingQueue {
public:

    void push(const T& v) {
        // auto now = std::chrono::steady_clock::now();
        // auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
        //     now.time_since_epoch()).count();
        
        //size_t queue_size = 0;
        {
            std::lock_guard<std::mutex> lock(mtx_);
            queue_.push(v);
            //queue_size = queue_.size();
        }
        
        // {
        //     std::lock_guard<std::mutex> log_lock(get_log_mutex());
        //     std::cout << "[BlockingQueue::push] ts=" << timestamp 
        //               << " us, queue_size=" << queue_size << std::flush << std::endl;
        // }
        
        cv_.notify_one();
    }

    bool pop(T& v) {
        // auto wait_start = std::chrono::steady_clock::now();
        // auto wait_start_ts = std::chrono::duration_cast<std::chrono::microseconds>(
        //     wait_start.time_since_epoch()).count();
        // {
        //     std::lock_guard<std::mutex> log_lock(get_log_mutex());
        //     std::cout << "[BlockingQueue::pop start] ts=" << wait_start_ts << " us" << std::flush << std::endl;
        // }
        std::unique_lock<std::mutex> lock(mtx_);
       

        cv_.wait(lock, [&] {
            return !queue_.empty() || stopped_;
        });

        // size_t queue_size_before = queue_.size();
        // bool was_empty = queue_.empty();


        // auto wait_end = std::chrono::steady_clock::now();
        // auto wait_end_ts = std::chrono::duration_cast<std::chrono::microseconds>(
        //     wait_end.time_since_epoch()).count();
        // auto wait_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        //     wait_end - wait_start).count();

        if (stopped_) {
            // {
            //     std::lock_guard<std::mutex> log_lock(get_log_mutex());
            //     std::cout << "[BlockingQueue::pop] ts=" << wait_end_ts 
            //               << " us, stopped=true" << std::flush << std::endl;
            // }
            return false;
        }

        v = std::move(queue_.front());
        queue_.pop();
        //size_t queue_size_after = queue_.size();
        
        // {
        //     std::lock_guard<std::mutex> log_lock(get_log_mutex());
        //     std::cout << "[BlockingQueue::pop] ts=" << wait_end_ts 
        //               << " us, wait_duration=" << wait_duration 
        //               << " us, was_empty=" << (was_empty ? "true" : "false")
        //               << ", queue_size_before=" << queue_size_before
        //               << ", queue_size_after=" << queue_size_after << std::flush << std::endl;
        // }
        
        return true;
    }

    bool tryPop(T& v) {
        std::lock_guard<std::mutex> lock(mtx_);
        
        if (queue_.empty() || stopped_) {
            return false;
        }
        
        v = std::move(queue_.front());
        // {
        //     std::lock_guard<std::mutex> log_lock(get_log_mutex());
        //     auto now = std::chrono::steady_clock::now();
        //     auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
        //     now.time_since_epoch()).count();

        //     size_t  queue_size = queue_.size();
        //     std::cout << "[BlockingQueue::tryPop] ts=" << timestamp 
        //               << " us, queue_size=" << queue_size << std::flush << std::endl;
        // }
        queue_.pop();
        return true;
    }
    
    void stop() {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            stopped_ = true;
        }
        cv_.notify_all();
    }

private:
    std::queue<T> queue_;
    std::mutex mtx_;
    std::condition_variable cv_;
    bool stopped_{false};
};

}

