#pragma once

#include <variant>
#include <vector>
#include <string>

namespace datapack {

template <typename Variant>
const std::vector<std::string>& variant_labels();

template <typename Variant, typename T>
const char* variant_label();

} // namespace datapack
