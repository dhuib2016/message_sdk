#include "zmq_base.h"

#include <chrono>

namespace msgsdk {

ZmqBase::ZmqBase(const ClientConfig &cfg)
    : config_(cfg), context_(), workers_(cfg.worker_threads) {}

ZmqBase::~ZmqBase() { stop(); }

bool ZmqBase::start() {

  if (running_)
    return true;

  socket_ = std::make_unique<zmqpp::socket>(context_, socketType());

  setupSocket(*socket_);

  // 连接
  // socket_->connect(config_.endpoint);
  // connect 只给客户端用
  if (socketType() == zmqpp::socket_type::req ||
      socketType() == zmqpp::socket_type::dealer ||
      socketType() == zmqpp::socket_type::sub ||
      socketType() == zmqpp::socket_type::push) {

    socket_->connect(config_.endpoint);
    // 等待连接建立（REQ/DEALER socket 需要）
    if (socketType() == zmqpp::socket_type::req ||
        socketType() == zmqpp::socket_type::dealer) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  workers_.start();

  running_ = true;

  // 对于 REQ socket，不需要 ioLoop（发送和接收都在 onSend 中同步完成）
  if (socketType() != zmqpp::socket_type::req) {

    wakeup_receiver_ =
        std::make_unique<zmqpp::socket>(context_, zmqpp::socket_type::pair);
    wakeup_sender_ =
        std::make_unique<zmqpp::socket>(context_, zmqpp::socket_type::pair);

    wakeup_receiver_->bind("inproc://wakeup");
    wakeup_sender_->connect("inproc://wakeup");

    io_thread_ = std::thread(&ZmqBase::ioLoop, this);
  }

  std::thread(&ZmqBase::dispatchLoop, this).detach();

  return true;
}

void ZmqBase::stop() {

  if (!running_)
    return;

  running_ = false;

  recv_queue_.stop();
  send_queue_.stop(); // 也要停止发送队列

  workers_.stop();

  if (io_thread_.joinable())
    io_thread_.join();

  if (socket_) {
    socket_->close();
    socket_.reset();
  }

  // context_.terminate();
}

bool ZmqBase::isRunning() const { return running_; }

void ZmqBase::setupSocket(zmqpp::socket &sock) {

  // 对于 REQ socket，receive_timeout 应该设为 -1（无限等待）
  // 因为 REQ socket 发送后必须阻塞等待回复
  if (socketType() == zmqpp::socket_type::req) {
    sock.set(zmqpp::socket_option::receive_timeout, -1); // 无限等待
    // sock.set(zmqpp::socket_option::identity, "ZMQ");
  } else {
    sock.set(zmqpp::socket_option::receive_timeout, config_.recv_timeout);
  }

  sock.set(zmqpp::socket_option::send_timeout, config_.send_timeout);

  sock.set(zmqpp::socket_option::send_high_water_mark, config_.send_hwm);

  sock.set(zmqpp::socket_option::receive_high_water_mark, config_.recv_hwm);
}

void ZmqBase::handleRawMessage(zmqpp::message &msg) {
  // 默认什么都不做
  (void)msg;
}

void ZmqBase::ioLoop() {

  zmqpp::poller poller;
  poller.add(*socket_);
  poller.add(*wakeup_receiver_);

  while (running_) {
    poller.poll(100); // 无限等待（真正事件驱动）

    if (!running_)
      break;

    // 1️⃣ 处理 wakeup
    if (poller.has_input(*wakeup_receiver_)) {

      zmqpp::message dummy;
      wakeup_receiver_->receive(dummy);

      Message msg;
      while (send_queue_.tryPop(msg)) {

        zmqpp::message zmsg;
        encode(msg, zmsg);

        std::lock_guard<std::mutex> lock(socket_mutex_);
        socket_->send(zmsg);
      }
    }

    // 2️⃣ 处理网络接收
    if (poller.has_input(*socket_)) {

      zmqpp::message zmsg;

      bool received = false;
      {
        std::lock_guard<std::mutex> lock(socket_mutex_);
        received = socket_->receive(zmsg);
      }

      if (received) {
        Message m;
        if (decode(zmsg, m)) {
          if (socketType() == zmqpp::socket_type::rep) {
            // REP socket: must send reply before next recv.
            // Call handler directly on io_thread to keep strict recv→send
            // order. The handler (e.g. rep_server.cpp) calls
            // server->send(reply), which invokes onSend() → send_queue_.push()
            // + wakeup_sender_. We then drain that reply here and send it
            // before polling again.
            if (handler_) {
              handler_(m); // handler calls server->send(reply) synchronously
            }
            // Now drain the reply that the handler put in send_queue_ via
            // onSend().
            Message reply_msg;
            while (send_queue_.tryPop(reply_msg)) {
              zmqpp::message reply_zmsg;
              encode(reply_msg, reply_zmsg);
              std::lock_guard<std::mutex> lock(socket_mutex_);
              socket_->send(reply_zmsg);
            }
            // Drain the wakeup signal that onSend() also sent so it doesn't
            // accumulate in the PAIR socket buffer.
            {
              zmqpp::message dummy;
              while (wakeup_receiver_->receive(dummy, true)) {
                // non-blocking drain
              }
            }
          } else {
            recv_queue_.push(m);
          }
        }
      }
    }
  }
}

void ZmqBase::dispatchLoop() {

  while (running_) {

    Message msg;

    if (!recv_queue_.pop(msg))
      break;

    if (handler_) {
      // auto now = std::chrono::steady_clock::now();
      // auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
      //     now.time_since_epoch()).count();
      // {
      //     std::lock_guard<std::mutex> log_lock(msgsdk::get_log_mutex());
      //     std::cout << "[handler real start] ts=" << timestamp << " us" <<
      //     std::flush << std::endl;
      // }
      // handler_(msg);
      workers_.post([=] { handler_(msg); });
    }
  }
}

bool ZmqBase::decode(zmqpp::message &zmsg, Message &msg) {

  try {
    if (socketType() == zmqpp::socket_type::router) {
      if (zmsg.parts() >= 3) {
        zmsg >> msg.identity >> msg.topic >> msg.payload;
      } else if (zmsg.parts() == 2) {
        zmsg >> msg.identity >> msg.payload;
      }
    } else {
      if (zmsg.parts() >= 2) {
        zmsg >> msg.topic >> msg.payload;
      } else {
        zmsg >> msg.payload;
      }
    }

    return true;
  } catch (...) {
    return false;
  }
}

void ZmqBase::encode(const Message &msg, zmqpp::message &zmsg) {
  if (socketType() == zmqpp::socket_type::router) {
    zmsg << msg.identity << msg.topic << msg.payload;
  } else {
    zmsg << msg.topic << msg.payload;
  }
}

void ZmqBase::onSend(const Message &msg) {
  // 对于 REQ socket，直接同步发送和接收
  if (socketType() == zmqpp::socket_type::req) {
    std::lock_guard<std::mutex> lock(socket_mutex_);

    try {
      if (!socket_) {
        std::cerr << "REQ socket is null" << std::endl;
        return;
      }

      // 发送请求
      zmqpp::message zmsg;
      encode(msg, zmsg);
      socket_->send(zmsg);

      // zmqpp::message request;
      // request << "Hello";
      // socket_->send(request);

      // 立即接收回复（阻塞，因为 receive_timeout 已设为 -1）
      zmqpp::message reply;
      bool received = socket_->receive(reply);
      if (received) {
        Message m;
        if (decode(reply, m)) {
          handler_(m);
          // recv_queue_.push(m);
        }
      }
    } catch (const zmqpp::zmq_internal_exception &e) {
      std::cerr << "REQ socket error: " << e.what() << std::endl;
    }
  } else {
    send_queue_.push(msg);

    // 唤醒 ioLoop (ZMQ sockets are not thread-safe, must lock it!)
    zmqpp::message dummy;
    dummy << 1;
    std::lock_guard<std::mutex> lock(socket_mutex_);
    // Use dont_wait so we don't block if the wakeup queue is full.
    // If it's full, a wakeup is already pending and ioLoop will drain the
    // queue.
    wakeup_sender_->send(dummy, true); // true = dont_wait
  }
}

} // namespace msgsdk
