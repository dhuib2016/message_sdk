#include <zmqpp/zmqpp.hpp>
#include <iostream>

int main() {
    zmqpp::context ctx;
    zmqpp::socket s(ctx, zmqpp::socket_type::pair);
    std::cout << "OK\n";
}
