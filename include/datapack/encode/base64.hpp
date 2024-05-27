#pragma once

#include <vector>
#include <string>
#include <cstdint>


namespace datapack {

std::string base64_encode(const std::vector<std::uint8_t>& data);
std::vector<std::uint8_t> base64_decode(const std::string& text);

std::size_t base64_encoded_length(const std::vector<std::uint8_t>& data);
std::size_t base64_decoded_length(const std::string& text);

class Base64Exception: public std::exception {
public:
    Base64Exception(const std::string& message):
        message(message)
    {}

private:
    const char* what() const noexcept override {
        return message.c_str();
    }
    std::string message;
};

} // namespace datapack
