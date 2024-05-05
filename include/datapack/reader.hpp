#pragma once

#include <type_traits>
#include <optional>
#include <concepts>
#include <string>
#include <variant>
#include <stdexcept>
#include "datapack/variant.hpp"


namespace datapack {

class Reader;

template <typename T>
concept readable = requires(Reader& reader, T& value) {
    { read(reader, value) };
};

class ReadableObject {
public:
    virtual void read(Reader&) = 0;
};

class Reader {
public:
    template <readable T>
    void value(T& value) {
        read(*this, value);
    }

    void value(ReadableObject& value) {
        object_begin();
        value.read(*this);
        object_end();
    }

    template <readable T>
    void value(std::optional<T>& value) {
        if (optional_begin()) {
            value.emplace();
            this->value(value.value());
            optional_end();
        } else {
            value = std::nullopt;
        }
    }

    template <typename Variant, typename T, typename ...Remainder>
    void variant_value(Variant& value) {
        if (variant_begin(variant_label<Variant, T>())) {
            T variant_value;
            this->value(variant_value);
            value = variant_value;
            variant_end();
        } else {
            if constexpr(sizeof...(Remainder) != 0) {
                variant_value<Variant, Remainder...>(value);
            }
            if constexpr(sizeof...(Remainder) == 0) {
                error("No matchin variant");
            }
        }
    }

    template <typename ...Args>
    void value(std::variant<Args...>& value) {
        variant_value<std::variant<Args...>, Args...>(value);
    }

    // Error handling
    virtual void error(const std::string& error) {
        throw std::runtime_error(error);
    }

    // Primitive
    virtual void value_f64(double& value) = 0;
    virtual void value_i32(int& value) = 0;

    // Compound
    virtual bool optional_begin() = 0;
    virtual void optional_end() = 0;
    virtual bool variant_begin(const char* type) = 0;
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

inline void read(Reader& reader, int& value) {
    reader.value_i32(value);
}

inline void read(Reader& reader, double& value) {
    reader.value_f64(value);
}

} // namespace datapack
