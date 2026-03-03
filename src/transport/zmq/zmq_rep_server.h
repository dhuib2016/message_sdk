#pragma once

#include "zmq_base.h"
#include "zmq_base_actor.h"

namespace msgsdk {

class ZmqRepServer :
    //public ZmqBase,
    public ZmqBaseActor,
    public IRepServer {

public:

    explicit ZmqRepServer(const ClientConfig& cfg);

    ~ZmqRepServer() override = default;

    // ISender
    bool send(const Message& msg) override;

    // IReceiver
    void setMessageHandler(MessageHandler handler) override;

protected:

    zmqpp::socket_type socketType() const override;

    void setupSocket(zmqpp::socket& sock) override;
};

}

