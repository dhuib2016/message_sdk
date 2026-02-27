#include "zmq_pub_server.h"

namespace msgsdk {

ZmqPubServer::ZmqPubServer(const ClientConfig& cfg)
    : ZmqBase(cfg)
{
}

zmqpp::socket_type ZmqPubServer::socketType() const {
    return zmqpp::socket_type::publish;
}

void ZmqPubServer::setupSocket(zmqpp::socket& sock) {

    sock.bind(config_.endpoint);

    // 推荐配置
    sock.set(zmqpp::socket_option::linger, 0);
    sock.set(zmqpp::socket_option::send_high_water_mark, 1000000);

}

void ZmqPubServer::onSend(const Message& msg) {

    zmqpp::message zmsg;
    encode(msg, zmsg);

    socket_->send(zmsg);
}

// PUB 不接收
void ZmqPubServer::handleRawMessage(zmqpp::message&) {
}

}