#pragma once

#include <type_traits>
#include <optional>
#include <concepts>
#include <string>
#include <variant>
#include <array>
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
    // Generic

    template <readable T>
    void value(T& value) {
        read(*this, value);
    }

    void value(ReadableObject& value) {
        object_begin();
        value.read(*this);
        object_end();
    }

    // Error handling

    virtual void error(const std::string& error) {
        throw std::runtime_error(error);
    }

    // Numbers
    // - Implement for each integer/float signed/unsigned 32/64-bit type

    virtual void value_f32(float& value) = 0;
    virtual void value_f64(double& value) = 0;

    virtual void value_i32(std::int32_t& value) = 0;
    virtual void value_i64(std::int64_t& value) = 0;
    virtual void value_u32(std::uint32_t& value) = 0;
    virtual void value_u64(std::uint64_t& value) = 0;

    virtual void value_u8(std::uint8_t& value) = 0;

    // Strings
    // - Also handle C-strings for embedded systems

#ifndef EMBED
    virtual void value_string(std::string& value) = 0;
#endif
    virtual void value_cstring(char* data, std::size_t capacity, std::size_t& length) = 0;

    // Other primitives

    virtual void value_bool(bool& value) = 0;
    // TODO: Datetime

    // Optional

#ifndef EMBED

    virtual bool optional_begin() = 0;
    virtual void optional_end() = 0;

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

#endif

    // Variant
    // - Implement std::variant
    // - Must pass the list of types in the begin_variant
    // -

#ifndef EMBED

    virtual void variant_begin(const std::vector<std::string>& types) = 0;
    virtual bool variant_match(const char* type) = 0;
    virtual void variant_end() = 0;

    template <typename Variant, typename T, typename ...Remainder>
    void variant_value(Variant& value) {
        if (variant_match(variant_label<Variant, T>())) {
            T variant_value;
            this->value(variant_value);
            value = variant_value;
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
        using Variant = std::variant<Args...>;
        variant_begin(variant_labels<Variant>());
        variant_value<std::variant<Args...>, Args...>(value);
        variant_end();
    }

#endif

    // Binary
    // - Use for any contiguous block of data, which applies to any
    //   contiguous array/vector of "plain old data" (pod) types
    // - Note: May use array type instead, but this is less efficient
    // - In order to handle different containers and read into them,
    //   the binary reading must start with a "binary_size" step,
    //   and then read into the pre-allocated block of memory

    virtual void binary_begin() = 0;
    virtual std::size_t binary_size() = 0;
    virtual void binary_data(std::uint8_t* data) = 0;
    virtual void binary_end() = 0;

#ifndef EMBED
    template <typename T>
    void value_binary(std::vector<T>& vec) {
        static_assert(std::is_pod_v<T>);
        binary_begin();
        std::size_t size = binary_size();
        if (size % sizeof(T) != 0) {
            error("Incorrect binary size");
        }
        vec.resize(size / sizeof(T));
        binary_data((std::uint8_t*)vec.data());
        binary_end();
    }
    template <typename T, std::size_t N>
    void value_binary(std::array<T, N>& arr) {
        static_assert(std::is_pod_v<T>);
        binary_begin();
        std::size_t size = binary_size();
        if (arr.size() * sizeof(T) != size) {
            error("Incorrect binary size");
        }
        binary_data((std::uint8_t*)arr.data());
        binary_end();
    }
#endif
    template <typename T, std::size_t N>
    void value_binary(T* data, std::size_t capacity, T& data_size) {
        static_assert(std::is_pod_v<T>);
        binary_begin();
        std::size_t size = binary_size();
        if (size > sizeof(T) * capacity) {
            error("Insufficient array capacity");
        }
        if (size % sizeof(T) != 0) {
            error("Incorrect binary size");
        }
        data_size = size / sizeof(T);
        binary_data((std::uint8_t*)data);
        binary_end();
    }

    // Object
    // - Fixed set of ordered key/value pairs
    // - Values can be of any type

    virtual void object_begin() = 0;
    virtual void object_end() = 0;
    virtual void object_next(const char* key) = 0;

    // Tuple
    // - Fixed set of ordered values
    // - Values can be of any type

    virtual void tuple_begin() = 0;
    virtual void tuple_end() = 0;
    virtual bool tuple_next() = 0;

    // List
    // - Arbitrary list of values
    // - Values must be the same type
    // - This also handles arbitrary maps, which should be represented
    //   as a list of key/value pairs (as length-2 tuples)

    virtual void list_begin() = 0;
    virtual void list_end() = 0;
#ifndef EMBED
    virtual bool list_next(std::string& key) = 0;
#endif
    virtual bool list_next(char* key, size_t& length, size_t capacity) = 0;
};

inline void read(Reader& reader, int& value) {
    reader.value_i32(value);
}

inline void read(Reader& reader, double& value) {
    reader.value_f64(value);
}

} // namespace datapack
