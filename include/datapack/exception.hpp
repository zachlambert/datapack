#pragma once

#include <exception>
#include <string>

namespace datapack {

class LoadException: public std::exception {
public:
    LoadException(const std::string& message):
        message(message)
    {}

private:
    const char* what() const noexcept override {
        return message.c_str();
    }
    std::string message;
};

} // namespace datapack
