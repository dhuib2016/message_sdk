#pragma once
#include "zmq_base.h"

namespace msgsdk {

class ZmqSubClient : public ZmqBase, public IReceiver {
public:
    explicit ZmqSubClient(const ClientConfig& cfg);
    // IReceiver
    void setMessageHandler(MessageHandler handler) override;

protected:
    zmqpp::socket_type socketType() const override;

    void setupSocket(zmqpp::socket& sock) override;

    void onSend(const Message& msg) override;

    void handleRawMessage(zmqpp::message& msg) override;
   
};

}