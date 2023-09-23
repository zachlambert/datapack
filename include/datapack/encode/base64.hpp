#pragma once

#include <cstdint>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

namespace datapack {

std::string base64_encode(const std::span<const std::uint8_t>& data);
std::vector<std::uint8_t> base64_decode(const std::string& text);

std::size_t base64_encoded_length(const std::span<const std::uint8_t>& data);
std::size_t base64_decoded_length(const std::string& text);

class Base64Exception : public std::runtime_error {
public:
  Base64Exception(const std::string& message) : std::runtime_error(message) {}
};

} // namespace datapack
