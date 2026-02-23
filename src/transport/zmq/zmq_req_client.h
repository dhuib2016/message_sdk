#pragma once

#include "zmq_base.h"

namespace msgsdk {

class ZmqReqClient :
    public ZmqBase,
    public IReqClient {

public:

    explicit ZmqReqClient(const ClientConfig& cfg);

    ~ZmqReqClient() override = default;

    // ISender
    bool send(const Message& msg) override;

    // IReceiver
    void setMessageHandler(MessageHandler handler) override;

protected:

    zmqpp::socket_type socketType() const override;
};

}

