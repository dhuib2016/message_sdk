#pragma once

#include <zmqpp/zmqpp.hpp>
#include <functional>
#include <memory>
#include <atomic>

namespace msgsdk {

struct ClientConfig {
    std::string endpoint;
    int recv_timeout = 3000;
    int send_timeout = 3000;
    int recv_hwm = 1000;
    int send_hwm = 1000;
};

struct Message {
    std::string identity;
    std::string topic;
    std::string payload;
};

class ZmqBaseActor {
public:
    using Handler = std::function<void(const Message&)>;

    ZmqBaseActor(const ClientConfig& cfg,
                 zmqpp::socket_type type);

    ~ZmqBaseActor();

    bool start();
    void stop();

    bool send(const Message& msg);

    void setHandler(Handler h);

protected:
    void setupSocket(zmqpp::socket& sock);
    void encode(const Message& msg, zmqpp::message& zmsg);
    bool decode(zmqpp::message& zmsg, Message& msg);

private:
    ClientConfig config_;
    zmqpp::socket_type socket_type_;

    zmqpp::context context_;
    std::unique_ptr<zmqpp::actor> actor_;

    Handler handler_;
};

}