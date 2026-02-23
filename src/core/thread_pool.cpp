#include "thread_pool.h"

namespace msgsdk {

ThreadPool::ThreadPool(int n)
    : thread_num_(n) {}

ThreadPool::~ThreadPool() {
    stop();
}

void ThreadPool::start() {

    if (running_) return;

    running_ = true;

    for (int i = 0; i < thread_num_; ++i) {
        threads_.emplace_back(
            &ThreadPool::worker, this);
    }
}

void ThreadPool::stop() {

    if (!running_) return;

    running_ = false;
    queue_.stop();

    for (auto& t : threads_) {
        if (t.joinable())
            t.join();
    }

    threads_.clear();
}

void ThreadPool::post(std::function<void()> task) {
    queue_.push(std::move(task));
}

void ThreadPool::worker() {

    while (running_) {

        std::function<void()> task;

        if (!queue_.pop(task))
            break;

        task();
    }
}

}

