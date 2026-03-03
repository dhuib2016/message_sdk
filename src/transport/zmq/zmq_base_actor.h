#pragma once

#include <zmqpp/zmqpp.hpp>
#include <functional>
#include <memory>
#include <atomic>

#include "message/message.h"
#include "message/config.h"
#include "message/client.h"

namespace msgsdk {

class ZmqBaseActor {
public:
    using Handler = std::function<void(const Message&)>;

    ZmqBaseActor(const ClientConfig& cfg, zmqpp::socket_type type);

    ~ZmqBaseActor();

    bool start();
    void stop();

    bool send(const Message& msg);

    void setHandler(Handler h);

protected:
    virtual zmqpp::socket_type socketType() const = 0;
    virtual void setupSocket(zmqpp::socket& sock);

    void encode(const Message& msg, zmqpp::message& zmsg);
    bool decode(zmqpp::message& zmsg, Message& msg);

protected:

    ClientConfig config_;
    zmqpp::socket_type socket_type_;
    std::unique_ptr<zmqpp::socket> socket_;

    zmqpp::context context_;
    std::unique_ptr<zmqpp::actor> actor_;

    Handler handler_;
    //IReceiver::MessageHandler handler_;
};

}