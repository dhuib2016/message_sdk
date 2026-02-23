#pragma once

#include "message.h"

#include <unordered_map>
#include <functional>
#include <mutex>
#include <string>

namespace msgsdk {

/**
 * @brief Topic 路由处理器
 * 
 * 根据消息的 topic 字段，将消息路由到不同的处理函数。
 * 支持注册多个 topic 的处理函数，每个 topic 的处理逻辑相互独立。
 * 
 * 使用示例：
 * @code
 *   TopicRouter router;
 *   
 *   router.registerHandler("user.login", [](const Message& msg) {
 *       Message reply;
 *       reply.topic = msg.topic;
 *       reply.payload = "Login OK: " + msg.payload;
 *       return reply;
 *   });
 *   
 *   router.registerHandler("order.create", handleOrderCreate);
 *   
 *   // 处理消息
 *   Message reply = router.handle(request);
 * @endcode
 */
class TopicRouter {
public:
    /**
     * @brief Topic 处理函数类型
     * 
     * 接收一个 Message，返回处理后的 Message（通常是回复）
     */
    using TopicHandler = std::function<Message(const Message&)>;

    /**
     * @brief 默认处理函数类型
     * 
     * 当没有找到对应 topic 的处理函数时调用
     */
    using DefaultHandler = std::function<Message(const Message&)>;

    /**
     * @brief 构造函数
     */
    TopicRouter() = default;

    /**
     * @brief 析构函数
     */
    ~TopicRouter() = default;

    // 禁止拷贝和移动（如果需要可以启用）
    TopicRouter(const TopicRouter&) = delete;
    TopicRouter& operator=(const TopicRouter&) = delete;
    TopicRouter(TopicRouter&&) = delete;
    TopicRouter& operator=(TopicRouter&&) = delete;

    /**
     * @brief 注册 topic 处理函数
     * 
     * @param topic topic 名称
     * @param handler 处理函数
     */
    void registerHandler(const std::string& topic, TopicHandler handler) {
        std::lock_guard<std::mutex> lock(mutex_);
        handlers_[topic] = std::move(handler);
    }

    /**
     * @brief 取消注册 topic 处理函数
     * 
     * @param topic topic 名称
     */
    void unregisterHandler(const std::string& topic) {
        std::lock_guard<std::mutex> lock(mutex_);
        handlers_.erase(topic);
    }

    /**
     * @brief 设置默认处理函数
     * 
     * 当没有找到对应 topic 的处理函数时，会调用默认处理函数。
     * 如果没有设置默认处理函数，会返回错误消息。
     * 
     * @param handler 默认处理函数
     */
    void setDefaultHandler(DefaultHandler handler) {
        std::lock_guard<std::mutex> lock(mutex_);
        default_handler_ = std::move(handler);
    }

    /**
     * @brief 处理消息
     * 
     * 根据消息的 topic 字段，路由到对应的处理函数。
     * 
     * @param msg 输入消息
     * @return 处理后的消息（通常是回复）
     */
    Message handle(const Message& msg) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = handlers_.find(msg.topic);
        if (it != handlers_.end()) {
            // 找到对应的处理函数，调用它
            return it->second(msg);
        }
        
        // 没有找到对应的处理函数
        if (default_handler_) {
            // 使用默认处理函数
            return default_handler_(msg);
        }
        
        // 返回错误消息
        Message reply;
        reply.topic = msg.topic;
        reply.payload = "ERROR: Unknown topic: " + msg.topic;
        return reply;
    }

    /**
     * @brief 检查是否已注册某个 topic
     * 
     * @param topic topic 名称
     * @return true 如果已注册，false 否则
     */
    bool hasHandler(const std::string& topic) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return handlers_.find(topic) != handlers_.end();
    }

    /**
     * @brief 获取已注册的 topic 数量
     * 
     * @return 已注册的 topic 数量
     */
    size_t handlerCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return handlers_.size();
    }

    /**
     * @brief 清空所有注册的处理函数
     */
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        handlers_.clear();
        default_handler_ = nullptr;
    }

     /**
     * @brief 获取所有已注册的 topic 列表
     * 
     * @return 包含所有已注册 topic 的 vector
     */
    std::vector<std::string> getTopics() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::string> topics;
        topics.reserve(handlers_.size());
        
        for (const auto& pair : handlers_) {
            topics.push_back(pair.first);
        }
        
        return topics;
    }

private:
    std::unordered_map<std::string, TopicHandler> handlers_;
    DefaultHandler default_handler_;
    mutable std::mutex mutex_;  // 保护 handlers_ 和 default_handler_
};

} // namespace msgsdk