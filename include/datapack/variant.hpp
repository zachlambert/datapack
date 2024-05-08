#pragma once

#include <variant>
#include <vector>
#include <string>
#include <any>
#include <unordered_map>

namespace datapack {

template <typename Variant>
const std::vector<std::string>& variant_labels();

template <typename Variant, typename T>
const char* variant_label();

// TODO: Dynamic variant?
// - Maintain a static unordered_map<string, any>
// - Load at runtime the set of possible types
// - Probably need to use a unique_ptr to Object (with read and write)
//   instead of any in order to read/write the value
// - Can create a "wrapper class" for primitives if necessary

} // namespace datapack
