#include "datapack/encode/floating_string.hpp"
#include <array>
#include <charconv>

namespace datapack {

std::string floating_to_string(float value) { return floating_to_string(double(value)); }

std::string floating_to_string(double value) {
  // Maximum number of values need to represent a double
  std::array<char, 24> string;
  auto res = std::to_chars(string.begin(), string.end(), value, std::chars_format::fixed);
  return std::string(string.data(), res.ptr - string.data());
}

} // namespace datapack
