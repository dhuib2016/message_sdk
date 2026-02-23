#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <csignal>
#include <unistd.h>

#include "transport/zmq/zmq_rep_server.h"
#include "transport/zmq/zmq_req_client.h"

using namespace msgsdk;

std::atomic<bool> g_running{true};

void signalHandler(int signal) {
    (void)signal;  // 标记为故意未使用
    g_running = false;
}

int main() {
    // 注册信号处理
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // 1. 启动 Server
    ClientConfig server_cfg;
    server_cfg.endpoint = "tcp://*:5556";
    server_cfg.worker_threads = 2;

    auto server = std::make_shared<ZmqRepServer>(server_cfg);

    server->setMessageHandler(
        [server](const Message& msg) {
            std::cout << "[Server] 收到: "
                      << msg.payload << std::endl;

            Message reply;
            reply.topic = msg.topic;
            reply.payload = "ACK: " + msg.payload;

            server->send(reply);
        });

    server->start();
    std::cout << "[Server] REP Server started on tcp://*:5556 (PID: " << getpid() << ")\n";

    // 2. 等待3秒
    std::cout << "[Main] 等待 3 秒后启动 Client...\n";
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // 3. 启动 Client 并发送消息
    ClientConfig client_cfg;
    client_cfg.endpoint = "tcp://127.0.0.1:5555";

    auto client = std::make_shared<ZmqReqClient>(client_cfg);

    client->setMessageHandler(
        [](const Message& msg) {
            std::cout << "[Client] 收到回复: "
                      << msg.payload << std::endl;
        });

    client->start();
    std::cout << "[Client] REQ Client started and connected\n";

    // 发送消息
    for (int i = 0; i < 5 && g_running; i++) {
        Message m;
        m.topic = "test";
        m.payload = "Hello " + std::to_string(i);

        std::cout << "[Client] 发送: " << m.payload << std::endl;
        client->send(m);

        std::this_thread::sleep_for(
            std::chrono::seconds(1));
    }

    // 保持运行，直到收到信号
    std::cout << "[Main] Server and Client running. Press Ctrl+C to exit...\n";
    while (g_running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "\n[Main] Shutting down...\n";
    client->stop();
    server->stop();

    return 0;
}