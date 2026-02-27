#include <iostream>
#include <chrono>
#include <atomic>
#include <memory>
#include "transport/zmq/zmq_sub_client.h"

using namespace msgsdk;

int main(int argc, char** argv)
{
    size_t expected_count = 1000000;

    if (argc > 1) expected_count = std::stoull(argv[1]);

    ClientConfig cfg;
    cfg.endpoint = "tcp://localhost:5555";

    auto client = std::make_shared<ZmqSubClient>(cfg);

    std::atomic<size_t> recv_count{0};
    std::chrono::steady_clock::time_point start_time;

    client->setMessageHandler([&](const Message& msg)
    {
        if (recv_count == 0)
            start_time = std::chrono::steady_clock::now();

        ++recv_count;
        std::cout << "recv_count:" << recv_count << std::endl;
        if (recv_count == expected_count)
        {
            auto end = std::chrono::steady_clock::now();
            double seconds =
                std::chrono::duration_cast<std::chrono::duration<double>>(end - start_time).count();

            double mps = recv_count / seconds;
            double mbps =
                (recv_count * msg.payload.size()) /
                (1024.0 * 1024.0) / seconds;

            std::cout << "\n=== SUB RESULT ===\n";
            std::cout << "Time: " << seconds << " sec\n";
            std::cout << "Rate: " << mps << " msg/s\n";
            std::cout << "Throughput: " << mbps << " MB/s\n";

            exit(0);
        }
    });

    if (!client->start()) {
        std::cerr << "SUB start failed\n";
        return -1;
    }

    std::cout << "SUB benchmark started...\n";

    while (true)
        std::this_thread::sleep_for(std::chrono::seconds(10));
}