#include "zmq_rep_server.h"

namespace msgsdk {

ZmqRepServer::ZmqRepServer(const ClientConfig& cfg)
    : ZmqBase(cfg) {
}

zmqpp::socket_type
ZmqRepServer::socketType() const {

    return zmqpp::socket_type::rep;
}

void ZmqRepServer::setupSocket(zmqpp::socket& sock) {

    ZmqBase::setupSocket(sock);

    // REP 需要 bind
    sock.bind(config_.endpoint);
}

bool ZmqRepServer::send(const Message& msg) {

    if (!running_) return false;

    try {
        onSend(msg);
        return true;
    }
    catch (...) {
        return false;
    }
}

void ZmqRepServer::setMessageHandler(
    MessageHandler handler) {

    handler_ = std::move(handler);
}

}

