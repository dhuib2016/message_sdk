#include <iostream>
#include <chrono>
#include <thread>
#include <memory>
#include <atomic>
#include "transport/zmq/zmq_pub_server.h"

using namespace msgsdk;

int main(int argc, char** argv)
{
    size_t message_count = 1000000;
    size_t message_size  = 100;   // bytes

    if (argc > 1) message_count = std::stoull(argv[1]);
    if (argc > 2) message_size  = std::stoull(argv[2]);
    std::cout << "message count:" << message_count << std::endl;
    std::cout << "message size:" << message_size << std::endl;
    ClientConfig cfg;
    cfg.endpoint = "tcp://*:5555";

    auto server = std::make_shared<ZmqPubServer>(cfg);

    if (!server->start()) {
        std::cerr << "PUB start failed\n";
        return -1;
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::string payload(message_size, 'A');

    std::cout << "PUB benchmark started\n";
    std::cout << "Messages: " << message_count
              << "  Size: " << message_size << " bytes\n";

    auto start = std::chrono::steady_clock::now();

    for (size_t i = 0; i < message_count; ++i)
    {
        Message msg;
        msg.topic = "bench";
        msg.payload = payload;

        server->send(msg);
        std::cout << "send_count:" << i+1 << std::endl;
        //std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    auto end = std::chrono::steady_clock::now();
    double seconds =
        std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();

    double mps = message_count / seconds;
    double mbps = (message_count * message_size) / (1024.0 * 1024.0) / seconds;

    std::cout << "\n=== PUB RESULT ===\n";
    std::cout << "Time: " << seconds << " sec\n";
    std::cout << "Rate: " << mps << " msg/s\n";
    std::cout << "Throughput: " << mbps << " MB/s\n";

    return 0;
}