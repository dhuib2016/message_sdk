#pragma once

#include "message.h"
#include "config.h"

#include <functional>
#include <memory>

namespace msgsdk {

//基础生命周期接口
class IMessageClient {
public:
    virtual ~IMessageClient() = default;

    // 启停（内部线程）
    virtual bool start() = 0;
    virtual void stop() = 0;

    // 连接状态
    virtual bool isRunning() const = 0;
};

//发送接口
class ISender {
public:
    virtual ~ISender() = default;

    // 同步发送
    virtual bool send(const Message& msg) = 0;
};

//接收接口（异步回调）
class IReceiver {
public:
    using MessageHandler =
        std::function<void(const Message&)>;

    virtual ~IReceiver() = default;

    // 注册回调
    virtual void setMessageHandler(
        MessageHandler handler) = 0;
};

//REQ Client
class IReqClient :
    //public IMessageClient,
    public ISender,
    public IReceiver {
};

//REP Server
class IRepServer :
    //public IMessageClient,
    public ISender,
    public IReceiver {
};

//PUB
class IPublisher :
    //public IMessageClient,
    public ISender {
};

//SUB
class ISubscriber :
    //public IMessageClient,
    public IReceiver {

public:
    virtual bool subscribe(const std::string& topic) = 0;
};

// PUSH / PULL
class IPusher :
    //public IMessageClient,
    public ISender {
};

class IPuller :
    //public IMessageClient,
    public IReceiver {
};


}

