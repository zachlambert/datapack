#pragma once

#include <variant>
#include <string>
#include <optional>
#include <vector>

namespace datapack{

namespace token {

using Binary = std::vector<std::uint8_t>;
// TODO: Date/Time/Datetime

using Primitive = std::variant<
    double,
    int,
    std::string,
    bool,
    std::nullopt_t,
    Binary
>;

struct Optional {
    bool has_value;
};

struct Variant {
    std::string type;
};

struct ObjectBegin {};
struct ObjectEnd {};
struct ObjectElement {
    std::string key;
    ObjectElement(const std::string& key):
        key(key)
    {}
};

struct ArrayBegin {};
struct ArrayEnd {};
struct ArrayElement {};

struct DocumentEnd {};

} // namespace token

using Token = std::variant<
    token::Primitive,
    token::Optional,
    token::Variant,
    token::ObjectBegin,
    token::ObjectEnd,
    token::ObjectElement,
    token::ArrayBegin,
    token::ArrayEnd,
    token::ArrayElement,
    token::DocumentEnd
>;

} // namespace datapack
