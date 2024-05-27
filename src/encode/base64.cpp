#include "datapack/encode/base64.hpp"
#include <stdexcept>

namespace datapack {

// https://en.wikipedia.org/wiki/Base64
// Using the base-64 alphabet defined in RFC 4648
// 0 -> 25 = A -> Z
// 26 -> 51 = a -> z
// 52 -> 61 = 0 -> 9
// 62 = +
// 63 = /
static std::uint8_t encode_symbol(std::uint8_t value) {
    if (value < 26) {
        return 'A' + (value - 0);
    }
    if (value < 52) {
        return 'a' + (value - 26);
    }
    if (value < 62) {
        return '0' + (value - 52);
    }
    if (value == 62) {
        return '+';
    }
    if (value == 63) {
        return '/';
    }
    throw std::runtime_error("Invalid value");
    return 0x00;
}

static std::uint8_t decode_symbol(std::uint8_t symbol) {
    if ('A' <= symbol && symbol <= 'Z') {
        return 0 + (symbol - 'A');
    }
    if ('a' <= symbol && symbol <= 'z') {
        return 26 + (symbol - 'a');
    }
    if ('0' <= symbol && symbol <= '9') {
        return 52 + (symbol - '0');
    }
    if (symbol == '+') {
        return 62;
    }
    if (symbol == '/') {
        return 63;
    }
    throw std::runtime_error("Invalid symbol");
    return 0x00;
}

std::string base64_encode(const std::vector<std::uint8_t>& data) {
    std::string text;
    std::size_t pos = 0;
    std::size_t i = 0;
    while (i < data.size()) {
        std::uint8_t a = data[i];
        std::uint8_t b = (i + 1 < data.size() ? data[i+1] : 0x00);
        std::uint8_t value = ((a >> pos) | (b << (8 - pos))) & 0x3F;
        text += encode_symbol(value);
        pos += 6;
        if (pos >= 8) {
            pos -= 8;
            i++;
        }
    }
    while (pos != 0) {
        text += "=";
        pos += 6;
        if (pos >= 8) {
            pos -= 8;
        }
    }
    return text;
}

std::vector<std::uint8_t> base64_decode(const std::string& text) {
    std::vector<std::uint8_t> data;
    std::size_t pos = 0;
    std::size_t i = 0;
    while (i < text.size() && text[i] != '=') {
        if (i + 1 >= text.size() || text[i+1] == '=') {
            break;
        }
        std::uint8_t a = decode_symbol(text[i]);
        std::uint8_t b = decode_symbol(text[i+1]);
        std::uint8_t value = (a >> pos) | (b << (6 - pos));
        data.push_back(value);
        pos += 8;
        while (pos >= 6) {
            pos-=6;
            i++;
        }
    }
    return data;
}

} // namespace datapack
