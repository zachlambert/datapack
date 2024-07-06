#pragma once

#ifndef EMBEDDED

#include <string>
#include <vector>
#include <optional>
#include <variant>

namespace datapack {

using string_t = std::string;
template <typename T>
using vector_t = std::vector<T>;
template <typename T>
using optional_t = std::optional<T>;
template <typename ...Args>
using variant_t = std::variant<Args...>;

}

#else

#include <micro_types/string.hpp>
#include <micro_types/vector.hpp>
#include <micro_types/optional.hpp>
#include <micro_types/variant.hpp>

namespace datapack {

using string_t = mct::string;
template <typename T>
using vector_t = mct::vector<T>;
template <typename T>
using optional_t = mct::optional<T>;
template <typename ...Args>
using variant_t = mct::variant<Args...>;

}

#endif
