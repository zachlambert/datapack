#include "datapack/encode/float_string.hpp"
#include <array>
#include <charconv>

namespace datapack {

template <typename T>
std::string float_to_string(T value) {
  // Maximum number of values need to represent a double
  std::array<char, 24> string;
  auto res = std::to_chars(string.begin(), string.end(), value, std::chars_format::fixed);
  return std::string(string.data(), res.ptr - string.data());
}
template std::string float_to_string<float>(float);
template std::string float_to_string<double>(double);

} // namespace datapack
