#pragma once
#include "zmq_base.h"

namespace msgsdk {

class ZmqPubServer : public ZmqBase {
public:
    explicit ZmqPubServer(const ClientConfig& cfg);

protected:
    zmqpp::socket_type socketType() const override;

    void setupSocket(zmqpp::socket& sock) override;

    void onSend(const Message& msg) override;

    void handleRawMessage(zmqpp::message& msg) override;
};

}