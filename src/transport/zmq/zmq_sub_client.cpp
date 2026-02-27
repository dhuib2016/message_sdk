#include "zmq_sub_client.h"

namespace msgsdk {

ZmqSubClient::ZmqSubClient(const ClientConfig& cfg)
    : ZmqBase(cfg)
{
}

zmqpp::socket_type ZmqSubClient::socketType() const {
    return zmqpp::socket_type::subscribe;
}

void ZmqSubClient::setupSocket(zmqpp::socket& sock) {

    sock.connect(config_.endpoint);

    // 订阅全部 topic
    sock.subscribe("");

    sock.set(zmqpp::socket_option::linger, 0);
    sock.set(zmqpp::socket_option::receive_high_water_mark, 1000000);
}

// SUB 不发送
void ZmqSubClient::onSend(const Message&) {
}

void ZmqSubClient::handleRawMessage(zmqpp::message& zmsg) {

    Message msg;

    if (decode(zmsg, msg)) {
        recv_queue_.push(msg);
    }
}

void ZmqSubClient::setMessageHandler(MessageHandler handler)
{
    handler_ = std::move(handler);
}

}