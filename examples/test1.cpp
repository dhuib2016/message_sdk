#include <iostream>
#include <string>
#include <zmqpp/zmqpp.hpp>

using namespace std;

int main()
{
    zmqpp::context ctx;

    auto lambda = [](zmqpp::socket* pipe)
    {
        pipe->send(zmqpp::signal::ok);
        pipe->send("xxxxxok");
        return true;
    };

    zmqpp::actor actor(lambda);

    zmqpp::message msg;
    actor.pipe()->receive(msg);
    std::cout << "OK\n";
    string a;
    msg >> a;
    std::cout << a << "\n";
}
