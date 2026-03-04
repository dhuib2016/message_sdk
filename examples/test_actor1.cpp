#include <iostream>
#include <string>
#include <zmqpp/zmqpp.hpp>

int main()
{
    // 定义 actor 线程逻辑
    auto lambda = [](zmqpp::socket* pipe)
    {
        zmqpp::message msg("boap");
        msg.add(42);
        msg.add(13);

        // 通知父线程启动成功
        pipe->send(zmqpp::signal::ok);

        // 发送真正消息
        pipe->send(msg);

        return true;  // 返回 true 会再发送 signal::ok 作为退出信号
    };

    // 创建 actor
    zmqpp::actor actor(lambda);

    // 接收 actor 发回的消息
    zmqpp::message msg;

    // 第一条是 signal::ok（自动发送）
    actor.pipe()->receive(msg);

    // 第二条是我们真正发送的数据
    //actor.pipe()->receive(msg);

    int a, b;
    std::string str;

    msg >> str >> a >> b;

    std::cout << "str = " << str << std::endl;
    std::cout << "a = " << a << std::endl;
    std::cout << "b = " << b << std::endl;

    if (str == "boap" && a == 42 && b == 13)
        std::cout << "Test PASS" << std::endl;
    else
        std::cout << "Test FAIL" << std::endl;

    return 0;
}
