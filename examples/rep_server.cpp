// 后台运行，输出重定向到文件
// nohup ./rep_server > server.log 2>&1 &

#include <iostream>

#include "transport/zmq/zmq_rep_server.h"
#include "message/topic_router.h"

using namespace msgsdk;

std::atomic<bool> g_running{true};

void signalHandler(int signal) {
    (void)signal;  // 标记为故意未使用
    g_running = false;
}

// 业务处理函数示例
msgsdk::Message handleUserLogin(const msgsdk::Message& msg) {
    //std::cout << "[UserLogin] Processing: " << msg.payload << std::endl;
    
    // 模拟耗时操作
    //std::this_thread::sleep_for(std::chrono::milliseconds(1));
    
    msgsdk::Message reply;
    reply.topic = msg.topic;
    reply.payload = "Login OK: " + msg.payload;
    return reply;
}

msgsdk::Message handleOrderCreate(const msgsdk::Message& msg) {
    std::cout << "[OrderCreate] Processing: " << msg.payload << std::endl;
    
    // 模拟耗时操作
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    
    msgsdk::Message reply;
    reply.topic = msg.topic;
    reply.payload = "Order Created: " + msg.payload;
    return reply;
}

msgsdk::Message handlePaymentProcess(const msgsdk::Message& msg) {
    std::cout << "[PaymentProcess] Processing: " << msg.payload << std::endl;
    
    // 模拟耗时操作
    //std::this_thread::sleep_for(std::chrono::milliseconds(150));
    
    msgsdk::Message reply;
    reply.topic = msg.topic;
    reply.payload = "Payment Processed: " + msg.payload;
    return reply;
}

// 默认处理函数
msgsdk::Message handleDefault(const msgsdk::Message& msg) {
    std::cout << "[Default] Unknown topic: " << msg.topic << std::endl;
    std::cout << "[Server] 收到payload: "
              << msg.payload << std::endl;

    msgsdk::Message reply;
    reply.topic = msg.topic;
    reply.payload = "ACK: " + msg.payload;
    return reply;
}


int main() {
    // 注册信号处理
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    ClientConfig cfg;
    //cfg.endpoint = "tcp://*:5555";
    cfg.endpoint = "ipc:///tmp/message_sdk.ipc";
    cfg.worker_threads = 2;

    auto server =
        std::make_shared<ZmqRepServer>(cfg);

    // 创建路由处理器
    msgsdk::TopicRouter router;
    
    // 注册不同 topic 的处理函数
    router.registerHandler("user.login", handleUserLogin);
    router.registerHandler("order.create", handleOrderCreate);
    router.registerHandler("payment.process", handlePaymentProcess);
    
    // 设置默认处理函数（可选）
    router.setDefaultHandler(handleDefault);

    server->setMessageHandler(
        //[server](const Message& msg){

        //std::cout << "[Server] 收到: "
        //         << msg.payload << std::endl;

        // Message reply;
        // reply.topic = msg.topic;
        // reply.payload = "ACK: " + msg.payload;
       
        [server, &router](const msgsdk::Message& msg) mutable {
            // 根据 topic 路由到对应的处理函数
            msgsdk::Message reply = router.handle(msg);

            server->send(reply);
        });

    server->start();

    std::cout << "REP Server running... (PID: " << getpid() << ")\n";
    //std::cout << "Registered topics: user.login, order.create, payment.process\n";
    // 获取所有已注册的 topic
    auto topics = router.getTopics();
    for (const auto& topic : topics) {    
        std::cout << "Registered topic: " << topic << std::endl;
    }
    std::cout << "Total handlers: " << router.handlerCount() << "\n";
    // 无限循环，等待信号
    while (g_running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "Shutting down server...\n";
    server->stop();

    return 0;
}

