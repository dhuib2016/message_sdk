#include "zmq_base_actor.h"
#include <iostream>

namespace msgsdk {

ZmqBaseActor::ZmqBaseActor(const ClientConfig& cfg,zmqpp::socket_type type)
    : config_(cfg), socket_type_(type)
{
}

ZmqBaseActor::~ZmqBaseActor() {
    stop();
}

bool ZmqBaseActor::start() {
    std::cout << "ZmqBaseActor::start()" << std::endl;
    if (actor_)
        return true;

    // auto cfg = config_;
    // auto type = socket_type_;

    actor_ = std::make_unique<zmqpp::actor>(
        [this](zmqpp::socket* pipe) -> bool {

       
        zmqpp::socket socket(context_, socket_type_);
        setupSocket(socket);
        try {
            // connect / bind
            // if (socketType() == zmqpp::socket_type::req ||
            //     socketType() == zmqpp::socket_type::dealer ||
            //     socketType() == zmqpp::socket_type::sub ||
            //     socketType() == zmqpp::socket_type::push)
            // {
            //     socket.connect(config_.endpoint);
            // }
            // else
            // {
            //     socket.bind(config_.endpoint);
            // }

            if (config_.bind) {
                std::cout << "bind " << config_.endpoint << std::endl;
                socket.bind(config_.endpoint);
            } else {
                std::cout << "connect " << config_.endpoint << std::endl;
                socket.connect(config_.endpoint);
            }

            // 告诉主线程初始化成功
            pipe->send(zmqpp::signal::ok);
        }
        catch (std::exception& e) {
            std::cout << "REAL ERROR: "
                      << e.what() << std::endl;

            return false;   // 让 actor 知道失败（初始化阶段出错）
        }

        // 进入事件循环，处理 socket 与 pipe 的数据
        #if 1
        zmqpp::poller poller;
        poller.add(socket);
        poller.add(*pipe);
        //std::cout << "poller.add(socket)" << std::endl;
        while (true) {

            poller.poll(100);

            // 1️⃣ 处理来自主线程的命令
            if (poller.has_input(*pipe)) {
                //std::cout << "poller.has_input(*pipe)" << std::endl;
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
                //std::cout << "has_input(socket)" << std::endl;
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
        #endif

        // 正常退出事件循环，返回 true
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

// void ZmqBase::setupSocket(zmqpp::socket& sock) {

//     // 对于 REQ socket，receive_timeout 应该设为 -1（无限等待）
//     // 因为 REQ socket 发送后必须阻塞等待回复
//     if (socketType() == zmqpp::socket_type::req) {
//         sock.set(zmqpp::socket_option::receive_timeout, -1);  // 无限等待
//         //sock.set(zmqpp::socket_option::identity, "ZMQ"); 
//     } else {
//         sock.set(zmqpp::socket_option::receive_timeout,
//                  config_.recv_timeout);
//     }

//     sock.set(zmqpp::socket_option::send_timeout,
//              config_.send_timeout);

//     sock.set(zmqpp::socket_option::send_high_water_mark,
//              config_.send_hwm);

//     sock.set(zmqpp::socket_option::receive_high_water_mark,
//              config_.recv_hwm);
// }


void ZmqBaseActor::encode(const Message& msg,
                          zmqpp::message& zmsg)
{
    if (socket_type_ == zmqpp::socket_type::router)
        zmsg << msg.topic << msg.payload;
    else
        zmsg << msg.topic << msg.payload;
}

bool ZmqBaseActor::decode(zmqpp::message& zmsg,
                          Message& msg)
{
    try {
        if (socket_type_ == zmqpp::socket_type::router) {
            if (zmsg.parts() >= 3)
                zmsg >>  msg.topic >> msg.payload;
            else if (zmsg.parts() == 2)
                zmsg >> msg.payload;
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