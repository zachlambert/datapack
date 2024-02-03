#pragma once

#include <vector>
#include <cstdint>
#include <exception>
#include <string>

namespace datapack {

class EncodingException: public std::exception {
public:
    EncodingException(const std::string& message):
        message(message)
    {}
private:
    const char* what() const noexcept override {
        return message.c_str();
    }
    std::string message;
};

using binary_t = std::vector<std::uint8_t>;

void base64_encode(const binary_t& from, const binary_t& to);
void base64_decode(const binary_t& from, const binary_t& to);

}

