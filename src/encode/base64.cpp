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
  throw Base64Exception("Invalid value");
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
  throw Base64Exception("Invalid symbol");
  return 0x00;
}

static std::uint8_t reverse_byte(std::uint8_t byte, int bits = 8) {
  std::uint8_t result = 0;
  for (std::size_t i = 0; i < bits; i++) {
    result |= ((byte & (1 << i)) >> i) << (bits - 1 - i);
  }
  return result;
}

std::string base64_encode(const std::vector<std::uint8_t>& data) {
  std::string text;
  std::size_t pos = 0;
  std::size_t i = 0;
  while (i < data.size()) {
    std::uint8_t a = reverse_byte(data[i]);
    std::uint8_t b = (i + 1 < data.size() ? reverse_byte(data[i + 1]) : 0x00);
    std::uint8_t value = ((a >> pos) | (b << (8 - pos))) & 0b00111111;
    text += encode_symbol(reverse_byte(value, 6));
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
    if (i + 1 >= text.size() || text[i + 1] == '=') {
      break;
    }
    std::uint8_t a = reverse_byte(decode_symbol(text[i]), 6);
    std::uint8_t b = reverse_byte(decode_symbol(text[i + 1]), 6);
    std::uint8_t value = (a >> pos) | (b << (6 - pos));
    data.push_back(reverse_byte(value));
    pos += 8;
    while (pos >= 6) {
      pos -= 6;
      i++;
    }
  }
  return data;
}

std::size_t base64_encoded_length(const std::vector<std::uint8_t>& data) {
  std::size_t blocks = data.size() / 3;
  if (data.size() % 3 != 0) {
    blocks += (3 - data.size() % 3);
  }
  return blocks * 4;
}

std::size_t base64_decoded_length(const std::string& text) {
  if (text.empty()) {
    return 0;
  }

  // Assume padding can be optional
  std::size_t full_blocks = text.length() / 4;
  if (text.length() % 4 == 0) {
    full_blocks--;
  }
  std::size_t last_block_i = full_blocks * 4;
  std::size_t length = full_blocks * 3;

  // Match the last 4 characters / padding to determine how many bytes
  // are encoded here
  // x=== (not possible)
  if (last_block_i + 1 >= text.size() || text[last_block_i + 1] == '=') {
    throw Base64Exception("Invalid amount of padding");
  }
  // xx== (1 byte)
  if (last_block_i + 2 >= text.size() || text[last_block_i + 2] == '=') {
    return length + 1;
  }
  // xxx= (2 bytes)
  if (last_block_i + 3 >= text.size() || text[last_block_i + 3] == '=') {
    return length + 2;
  }
  // xxxx (3 bytes)
  return length + 3;
}

} // namespace datapack
