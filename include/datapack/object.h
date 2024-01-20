#pragma once

#include <memory>
#include <variant>
#include <optional>
#include <vector>
#include <tsl/ordered_map.h>

#include "datapack/writer.h"
#include "datapack/reader.h"

namespace datapack {

class Value;
using null_t = std::nullopt_t;
using binary_t = std::vector<std::uint8_t>;
using Array = std::vector<Value>;
using Object = tsl::ordered_map<std::string, Value>;

using ValueType = std::variant<
    float,
    double,
    int,
    long,
    bool,
    null_t,
    binary_t,
    Array,
    Object
>;

struct Value {
public:
    template <typename... Args>
    Value(Args&& ...args):
        value(std::forward<Args...>(args...))
    {}
    template <typename T>
    Value& operator=(const T& other) {
        value = other;
        return *this;
    }
    template <typename T>
    Value& operator=(T&& other) {
        value = std::move(other);
        return *this;
    }

    template <typename T>
    const T* as() const {
        return std::get_if<T>(&value);
    }

    template <typename T>
    T* as() {
        return std::get_if<T>(&value);
    }

private:
    ValueType value;
};

} // namespace datapack
