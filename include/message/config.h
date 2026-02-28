#pragma once

#include <cstdint>
#include <string>


namespace msgsdk {

struct ClientConfig {

  // 网络参数
  std::string endpoint;

  int send_timeout = -1;  // 发送超时(ms)，-1表示无限等待
  int recv_timeout = 100; // 接收轮询超时(ms)

  int send_hwm = 0; // 发送高水位标记 (0 = unlimited)
  int recv_hwm = 0; // 接收高水位标记 (0 = unlimited)

  // 自动重连
  bool auto_reconnect = true;
  int reconnect_interval = 3000;
  int worker_threads = 1;

  int hwm = 1000;
  bool bind = false;
};

} // namespace msgsdk
