#include "zmq_rep_server.h"

namespace msgsdk {

ZmqRepServer::ZmqRepServer(const ClientConfig& cfg)
    : ZmqBaseActor(cfg,zmqpp::socket_type::rep) {
}

zmqpp::socket_type
ZmqRepServer::socketType() const {

    return zmqpp::socket_type::rep;
}

void ZmqRepServer::setupSocket(zmqpp::socket& sock) {

    ZmqBaseActor::setupSocket(sock);

    // REP 需要 bind
    //sock.bind(config_.endpoint);
}

bool ZmqRepServer::send(const Message& msg) {

    //if (!running_) return false;

    try {
        //onSend(msg);
        ZmqBaseActor::send(msg);
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

