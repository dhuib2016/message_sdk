#include <atomic>
#include <iostream>
#include <thread>

#include "transport/zmq/zmq_req_client.h"

using namespace msgsdk;

int main() {

  ClientConfig cfg;
  // cfg.endpoint = "tcp://127.0.0.1:5555";
  cfg.endpoint = "ipc:///tmp/message_sdk.ipc";

  auto client = std::make_shared<ZmqReqClient>(cfg);

  std::atomic<int> received_count{0};

  client->setMessageHandler([&received_count](const Message &msg) {
    (void)msg;
    received_count++;
  });

  client->start();

  // =============================
  // 10000 次调用的 benchmark 代码
  // =============================
  const int N = 50000; // 000;

  std::cout << "Benchmark: sending " << N << " requests..." << std::endl;

  auto t1 = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < N; ++i) {
    Message m;
    m.topic = "user.login"; // 或 "test"
    m.payload = "Hello " + std::to_string(i);
    client->send(m);
  }

  while (received_count < N) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  auto t2 = std::chrono::high_resolution_clock::now();

  auto total_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
  double avg_us = (total_ms * 1000.0) / N;

  std::cout << "Total time for " << N << " calls: " << total_ms
            << " ms, avg: " << avg_us << " us/call" << std::endl;

  client->stop();
  return 0;
}
