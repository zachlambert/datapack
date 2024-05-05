#pragma once

#include <type_traits>
#include <optional>
#include <concepts>
#include <string>
#include <variant>
#include "datapack/variant.h"


namespace datapack {

class Writer;

template <typename T>
concept writeable = requires(Writer& writer, const T& value) {
    { write(writer, value) };
};

class WriteableObject {
public:
    virtual void write(Writer&) const = 0;
};

class Writer {
public:
    template <writeable T>
    void value(T& value) {
        write(*this, value);
    }

    void value(const WriteableObject& value) {
        object_begin();
        value.write(*this);
        object_end();
    }

    template <writeable T>
    void value(const std::optional<T>& value) {
        optional_begin(value.has_value());
        if (value.has_value()) {
            this->value(value.value());
            optional_end();
        }
    }

    template <typename ...Args>
    void value(const std::variant<Args...>& value) {
        using Variant = std::variant<Args...>;
        std::visit([&](const auto& value) {
            using T = std::decay_t<decltype(value)>;
            variant_begin(variant_label<Variant, T>());
            this->value(value);
            variant_end();
        }, value);
    }

    // Primitive
    virtual void value_f64(const double& value) = 0;
    virtual void value_i32(const int& value) = 0;

    // Compound
    virtual void optional_begin(bool has_value) = 0;
    virtual void optional_end() = 0;
    virtual void variant_begin(const char* label) = 0;
    virtual void variant_end() = 0;

    // Container
    virtual void object_begin() = 0;
    virtual void object_end() = 0;
    virtual void object_next(const char* key) = 0;

#if 0
    virtual void array_begin() = 0;
    virtual void array_end() = 0;
    virtual bool array_next() = 0;

    virtual void map_begin() = 0;
    virtual void map_end() = 0;
    virtual bool map_next(std::string& key) = 0;
    virtual bool map_next(char* key, size_t& length, size_t capacity) = 0;
#endif
};

inline void write(Writer& writer, const int& value) {
    writer.value_i32(value);
}

inline void write(Writer& writer, const double& value) {
    writer.value_f64(value);
}

} // namespace datapack
