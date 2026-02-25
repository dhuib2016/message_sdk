#include <iostream>
#include <thread>
#include "transport/zmq/zmq_pub_server.h"

using namespace msgsdk;

int main() {

    ClientConfig cfg;
    cfg.endpoint = "tcp://*:5555";

    auto server = std::make_shared<ZmqPubServer>(cfg);

    if (!server->start()) {
        std::cerr << "PUB start failed\n";
        return -1;
    }

    std::cout << "PUB server started...\n";

    int i = 0;

    while (true) {

        Message msg;
        msg.topic = "market";
        msg.payload = "price=" + std::to_string(i++);

        server->send(msg);

        std::cout << "send: " << msg.payload << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}