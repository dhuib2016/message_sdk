#pragma once

#include <stdexcept>
#include <string>

namespace msgsdk {

class MessageException : public std::runtime_error {

public:
    MessageException(const std::string& msg,
                     const std::string& module = "")
        : std::runtime_error(msg),
          module_(module) {}

    const std::string& module() const {
        return module_;
    }

private:
    std::string module_;
};

}

