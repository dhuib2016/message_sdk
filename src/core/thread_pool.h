#pragma once

#include <vector>
#include <thread>
#include <functional>
#include <atomic>

#include "blocking_queue.h"

namespace msgsdk {

class ThreadPool {
public:

    explicit ThreadPool(int n = 1);

    ~ThreadPool();

    void start();

    void stop();

    void post(std::function<void()> task);

private:

    void worker();

    int thread_num_;

    std::vector<std::thread> threads_;

    BlockingQueue<std::function<void()>> queue_;

    std::atomic<bool> running_{false};
};

}

