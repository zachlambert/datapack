#pragma once

#include <vector>
#include <string>
#include <cstdint>


namespace datapack {

std::string base64_encode(const std::vector<std::uint8_t>& data);
std::vector<std::uint8_t> base64_decode(const std::string& text);

} // namespace datapack
