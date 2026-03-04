#include <iostream>
#include <thread>

#include "zmqpp/context.hpp"
#include "zmqpp/message.hpp"
#include "zmqpp/actor.hpp"
#include "zmqpp/poller.hpp"
#include <iostream>

using namespace std;

int main() {


    auto lambda = [](zmqpp::socket * pipe)
    {
        zmqpp::message msg("boap");
        msg.add(42);
        msg.add(13);
        pipe->send(zmqpp::signal::ok); // signal we successfully started
        pipe->send(msg);
        return true; // will send signal::ok to signal successful shutdown
    };
    zmqpp::actor actor(lambda);


    zmqpp::message msg;
    actor.pipe()->receive(msg);
    int a, b;
    std::string str;

    msg >> str >> a >> b;
    cout << "str:" << str <<endl;
    cout << "a:" << a <<endl;
    cout << "b:" << b <<endl;

    //cfg.endpoint = "tcp://127.0.0.1:5555";
}

