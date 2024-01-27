#pragma once

#include <memory>
#include <variant>
#include <optional>
#include <vector>

#include "datapack/token.h"
#include "datapack/writer.h"
#include "datapack/reader.h"

namespace datapack {

class ObjectWriter: public Writer {
public:
    ObjectWriter& key(const std::string& key) override;
    ObjectWriter& next() override;

    ObjectWriter& i32(int value) override;
    ObjectWriter& i64(long value) override;
    ObjectWriter& f32(float value) override;
    ObjectWriter& f64(double value) override;
    ObjectWriter& string(const std::string& value) override;
    ObjectWriter& boolean(bool value) override;
    ObjectWriter& null() override;
    ObjectWriter& binary(const binary_t& value) override;

    template <writeable T>
    ObjectWriter& value(const T& value) {
        write(*this, value);
        return *this;
    }

    ObjectWriter& start_array() override;
    ObjectWriter& end_array() override;

    ObjectWriter& start_object() override;
    ObjectWriter& end_object() override;

    const std::vector<Token> result() {
        return tokens;
    }

private:
    std::vector<Token> tokens;
};

// OLD: Keep for reference

#if 0
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

struct Value: public Writeable {
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

    void write(Writer& writer) const;

private:
    ValueType value;
};
#endif


} // namespace datapack
