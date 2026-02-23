#pragma once

#include "client.h"

#include <memory>
#include <string>

namespace msgsdk {

enum class ClientType {
    REQ,
    REP,
    PUB,
    SUB,
    PUSH,
    PULL
};

class MessagePlatform {
public:

    static MessagePlatform& instance();

    // 工厂创建
    std::shared_ptr<IMessageClient>
    createClient(ClientType type,
                 const ClientConfig& cfg,
                 const std::string& name);

    // 查找
    std::shared_ptr<IMessageClient>
    getClient(const std::string& name);

    // 删除
    void remove(const std::string& name);

    // 停止全部
    void shutdown();

private:
    MessagePlatform() = default;
};

}

