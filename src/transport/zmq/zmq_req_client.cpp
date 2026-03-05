#include "zmq_req_client.h"

namespace msgsdk {

ZmqReqClient::ZmqReqClient(const ClientConfig& cfg)
    : ZmqBaseActor(cfg,zmqpp::socket_type::req) {
}

zmqpp::socket_type
ZmqReqClient::socketType() const {

    return zmqpp::socket_type::req;
}

bool ZmqReqClient::send(const Message& msg) {

    //if (!running_) return false;

    try {
        ZmqBaseActor::send(msg);
        return true;
    }
    catch (...) {
        return false;
    }
}

void ZmqReqClient::setMessageHandler(
    MessageHandler handler) {

    handler_ = std::move(handler);
}

}

