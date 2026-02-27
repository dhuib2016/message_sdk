#include <iostream>
#include "transport/zmq/zmq_sub_client.h"

using namespace msgsdk;

int main() {

    ClientConfig cfg;
    cfg.endpoint = "tcp://localhost:5555";

    auto client = std::make_shared<ZmqSubClient>(cfg);

    client->setMessageHandler([](const Message& msg) {
        std::cout << "recv: "
                  << msg.topic << " -> "
                  << msg.payload
                  << std::endl;
    });

    if (!client->start()) {
        std::cerr << "SUB start failed\n";
        return -1;
    }

    std::cout << "SUB client started...\n";

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    return 0;
}