#pragma once

#include <string>
#include <cstdint>

namespace msgsdk {

struct Message {

    std::string topic;      // 主题
    std::string payload;    // 内容

    uint64_t seq = 0;       // 序号
    uint64_t timestamp = 0;// 时间戳(ms)

};

}

