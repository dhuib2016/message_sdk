#pragma once

#include <zmqpp/zmqpp.hpp>
#include <thread>
#include <atomic>
#include <memory>
#include <unistd.h>

#include "message/message.h"
#include "message/config.h"
#include "message/client.h"

#include "core/thread_pool.h"
#include "core/blocking_queue.h"

namespace msgsdk {

class ZmqBase : public IMessageClient {

public:

    explicit ZmqBase(const ClientConfig& cfg);

    virtual ~ZmqBase();

    bool start() override;

    void stop() override;

    bool isRunning() const override;

    bool send(const Message& msg);

protected:

    // 派生类必须实现
    virtual zmqpp::socket_type socketType() const = 0;

    virtual void setupSocket(zmqpp::socket& sock);

    virtual void onSend(const Message& msg);

    // 收到zmq原始消息
    virtual void handleRawMessage(zmqpp::message& msg);

    // 转换为Message
    virtual bool decode(zmqpp::message&, Message&);

    virtual void encode(const Message&, zmqpp::message&);

protected:

    ClientConfig config_;

    zmqpp::context context_;

    std::unique_ptr<zmqpp::socket> socket_;

    std::thread io_thread_;

    ThreadPool workers_;

    BlockingQueue<Message> recv_queue_;

    std::atomic<bool> running_{false};

    IReceiver::MessageHandler handler_;

    std::mutex socket_mutex_;

    BlockingQueue<Message> send_queue_;

    std::unique_ptr<zmqpp::socket> wakeup_receiver_;

    std::unique_ptr<zmqpp::socket> wakeup_sender_;

protected:

    void ioLoop();

    void dispatchLoop();
};

}

