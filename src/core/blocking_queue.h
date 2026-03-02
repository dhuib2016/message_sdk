#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

namespace msgsdk {

template<typename T>
class BlockingQueue {
public:

    void push(const T& v) {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            queue_.push(v);
        }
        cv_.notify_one();
    }

    bool pop(T& v) {
        std::unique_lock<std::mutex> lock(mtx_);

        cv_.wait(lock, [&] {
            return !queue_.empty() || stopped_;
        });

        if (stopped_) {
            return false;
        }

        v = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    bool tryPop(T& v) {
        std::lock_guard<std::mutex> lock(mtx_);
        
        if (queue_.empty() || stopped_) {
            return false;
        }
        
        v = std::move(queue_.front());
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

