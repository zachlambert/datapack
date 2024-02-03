#pragma once

#include <memory>
#include <variant>
#include <optional>
#include <vector>

#include "datapack/binary.h"
#include "datapack/reader.h"


namespace datapack {

using Primitive = std::variant<
    int,
    long,
    float,
    double,
    std::string,
    bool,
    std::nullopt_t,
    binary_t>;

struct ObjectBegin {};
struct ObjectEnd {};
struct ObjectElement {
    const std::string key;
    ObjectElement(const std::string& key): key(key) {}
};

struct ArrayBegin {};
struct ArrayEnd {};
struct ArrayElement {};

using Token = std::variant<
    Primitive,
    ObjectBegin, ObjectEnd, ObjectElement,
    ArrayBegin, ArrayEnd, ArrayElement
>;

} // namespace datapack
