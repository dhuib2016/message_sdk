#include "zmq_base_actor.h"
#include <iostream>

namespace msgsdk {

ZmqBaseActor::ZmqBaseActor(const ClientConfig& cfg,
                           zmqpp::socket_type type)
    : config_(cfg),
      socket_type_(type)
{
}

ZmqBaseActor::~ZmqBaseActor() {
    stop();
}

bool ZmqBaseActor::start() {

    if (actor_)
        return true;

    actor_ = std::make_unique<zmqpp::actor>(
        [this](zmqpp::socket* pipe) -> bool {

        zmqpp::socket socket(context_, socket_type_);
        setupSocket(socket);

        // connect / bind
        if (socket_type_ == zmqpp::socket_type::req ||
            socket_type_ == zmqpp::socket_type::dealer ||
            socket_type_ == zmqpp::socket_type::sub ||
            socket_type_ == zmqpp::socket_type::push)
        {
            socket.connect(config_.endpoint);
        }
        else
        {
            socket.bind(config_.endpoint);
        }

        pipe->send(zmqpp::signal::ok);

        zmqpp::poller poller;
        poller.add(socket);
        poller.add(*pipe);

        while (true) {

            poller.poll();

            // 1️⃣ 处理来自主线程的命令
            if (poller.has_input(*pipe)) {

                zmqpp::message cmd;
                pipe->receive(cmd);

                if (cmd.is_signal()) {
                    zmqpp::signal sig;
                    cmd.get(sig, 0);
                    if (sig == zmqpp::signal::stop)
                        break;
                }

                Message m;
                if (decode(cmd, m)) {

                    zmqpp::message zmsg;
                    encode(m, zmsg);

                    socket.send(zmsg);

                    // REQ 模式需要立即 recv
                    if (socket_type_ == zmqpp::socket_type::req) {
                        zmqpp::message reply;
                        if (socket.receive(reply)) {
                            Message rm;
                            if (decode(reply, rm) && handler_) {
                                handler_(rm);
                            }
                        }
                    }
                }
            }

            // 2️⃣ 处理网络数据
            if (poller.has_input(socket)) {

                zmqpp::message zmsg;
                if (socket.receive(zmsg)) {

                    Message m;
                    if (decode(zmsg, m)) {

                        if (socket_type_ == zmqpp::socket_type::rep) {
                            // REP 必须立即处理并回复
                            if (handler_)
                                handler_(m);
                        }
                        else {
                            if (handler_)
                                handler_(m);
                        }
                    }
                }
            }
        }

        return true;
    });

    return true;
}

void ZmqBaseActor::stop() {

    if (actor_) {
        actor_->stop(true);
        actor_.reset();
    }
}

bool ZmqBaseActor::send(const Message& msg) {

    if (!actor_)
        return false;

    zmqpp::message zmsg;
    encode(msg, zmsg);

    actor_->pipe()->send(zmsg);

    return true;
}

void ZmqBaseActor::setHandler(Handler h) {
    handler_ = std::move(h);
}

void ZmqBaseActor::setupSocket(zmqpp::socket& sock) {

    sock.set(zmqpp::socket_option::receive_timeout,
             config_.recv_timeout);

    sock.set(zmqpp::socket_option::send_timeout,
             config_.send_timeout);

    sock.set(zmqpp::socket_option::send_high_water_mark,
             config_.send_hwm);

    sock.set(zmqpp::socket_option::receive_high_water_mark,
             config_.recv_hwm);
}

void ZmqBaseActor::encode(const Message& msg,
                          zmqpp::message& zmsg)
{
    if (socket_type_ == zmqpp::socket_type::router)
        zmsg << msg.identity << msg.topic << msg.payload;
    else
        zmsg << msg.topic << msg.payload;
}

bool ZmqBaseActor::decode(zmqpp::message& zmsg,
                          Message& msg)
{
    try {
        if (socket_type_ == zmqpp::socket_type::router) {
            if (zmsg.parts() >= 3)
                zmsg >> msg.identity >> msg.topic >> msg.payload;
            else if (zmsg.parts() == 2)
                zmsg >> msg.identity >> msg.payload;
        }
        else {
            if (zmsg.parts() >= 2)
                zmsg >> msg.topic >> msg.payload;
            else
                zmsg >> msg.payload;
        }
        return true;
    }
    catch (...) {
        return false;
    }
}

}