#pragma once

#include <variant>
#include <string>

namespace datapack{

namespace token {

using Primitive = std::variant<
    double,
    int
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
};

} // namespace token

using Token = std::variant<
    token::Primitive,
    token::Optional,
    token::Variant,
    token::ObjectBegin,
    token::ObjectEnd,
    token::ObjectElement
>;

} // namespace datapack
