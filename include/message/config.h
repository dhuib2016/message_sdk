#pragma once

#include <string>
#include <cstdint>

namespace msgsdk {

struct ClientConfig {

    // 网络参数
    std::string endpoint;

    int send_hwm = 1000;
    int recv_hwm = 1000;

    int io_threads = 1;

    // 超时(ms)
    int recv_timeout = 1000;
    int send_timeout = 1000;

    // 自动重连
    bool auto_reconnect = true;
    int reconnect_interval = 3000;

    // 工作线程
    int worker_threads = 1;

    int hwm = 1000;
    bool bind = false;
};

}

