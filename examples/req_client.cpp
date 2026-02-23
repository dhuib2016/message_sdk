#include <iostream>
#include <thread>

#include "transport/zmq/zmq_req_client.h"

using namespace msgsdk;

int main() {

    ClientConfig cfg;
    //cfg.endpoint = "tcp://127.0.0.1:5555";
    cfg.endpoint = "ipc:///tmp/message_sdk.ipc";

    auto client =
        std::make_shared<ZmqReqClient>(cfg);

    client->setMessageHandler(
        [](const Message& msg){
        (void)msg;  // 标记为故意未使用

        //std::cout << "[Client] 回复: "
          //        << msg.payload << std::endl;
    });

    client->start();

    // for (int i = 0; i < 5; i++) {
    //     std::cout << "start send count:" << i << std::endl;
    //     Message m;
    //     m.topic = "";  //"test";
    //     m.payload = "Hello " + std::to_string(i);

    //     client->send(m);

    //     std::this_thread::sleep_for(
    //         std::chrono::seconds(1));
    // }

    // =============================
    // 10000 次调用的 benchmark 代码
    // =============================
    const int N = 50000;   //000;

    std::cout << "Benchmark: sending " << N << " requests..." << std::endl;

    auto t1 = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < N; ++i) {
        Message m;
        m.topic = "user.login";  // 或 "test"
        m.payload = "Hello " + std::to_string(i);

        client->send(m);

        //std::this_thread::sleep_for(
        //     std::chrono::seconds(1));
        // 注意：当前 ZmqReqClient::send 是同步 REQ（send+recv），
        // 每次 send 内部会阻塞等待回复，所以这里不需要再 sleep。
    }

    auto t2 = std::chrono::high_resolution_clock::now();

    auto total_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    double avg_us = (total_ms * 1000.0) / N;

    std::cout << "Total time for " << N << " calls: "
              << total_ms << " ms, avg: "
              << avg_us << " us/call" << std::endl;

    std::cin.get();
}

