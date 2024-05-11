#pragma once

#include <type_traits>
#include <optional>
#include <concepts>
#include <string>
#include <variant>
#include <array>
#include <stdexcept>
#include <unordered_map>
#include "datapack/enum.hpp"
#include "datapack/variant.hpp"

namespace datapack {

class Reader;

template <typename T>
concept readable = requires(Reader& reader, T& value) {
    { read(reader, value) };
};

class Readable {
public:
    virtual void read(Reader&) = 0;
};

inline void read(Reader& reader, Readable& value) {
    value.read(reader);
}

class Reader {
public:
    template <readable T>
    void value(T& value) {
        read(*this, value);
    }

    template <readable T>
    void value(const char* key, T& value) {
        object_next(key);
        this->value(value);
    }

    virtual void value_i32(std::int32_t& value) = 0;
    virtual void value_i64(std::int64_t& value) = 0;
    virtual void value_u32(std::uint32_t& value) = 0;
    virtual void value_u64(std::uint64_t& value) = 0;

    virtual void value_f32(float& value) = 0;
    virtual void value_f64(double& value) = 0;

    virtual void value_string(std::string& value) = 0;
    virtual void value_bool(bool& value) = 0;

    virtual int enumerate(const std::vector<const char*>& labels) = 0;
    virtual bool optional() = 0;
    virtual const char* variant(const std::vector<const char*>& labels) = 0;

    virtual std::size_t binary_size(std::size_t expected_size=0) = 0;
    virtual void binary_data(std::uint8_t* data) = 0;

    template <typename T>
    void value_binary(std::vector<T>& value, std::size_t expected_size=0) {
        static_assert(std::is_trivial_v<T>);
        std::size_t size = binary_size(expected_size * sizeof(T));
        if (size % sizeof(T) != 0) {
            error("Incorrect binary size");
        }
        value.resize(size / sizeof(T));
        binary_data((std::uint8_t*)value.data());
    }

    template <typename T, std::size_t N>
    void value_binary(std::array<T, N>& value) {
        static_assert(std::is_trivial_v<T>);
        std::size_t size = binary_size();
        if (value.size() * sizeof(T) != size) {
            error("Incorrect binary size");
        }
        binary_data((std::uint8_t*)value.data());
    }

    virtual void object_begin() = 0;
    virtual void object_end() = 0;
    virtual void object_next(const char* key) = 0;

    virtual void tuple_begin() = 0;
    virtual void tuple_end() = 0;
    virtual bool tuple_next() = 0;

    virtual void map_begin() = 0;
    virtual void map_end() = 0;
    virtual bool map_next(std::string& key) = 0;

    virtual void list_begin() = 0;
    virtual void list_end() = 0;
    virtual bool list_next() = 0;

protected:
    virtual void error(const std::string& error) {
        throw std::runtime_error(error);
    }
};

inline void read(Reader& reader, std::int32_t& value) {
    reader.value_i32(value);
}

inline void read(Reader& reader, std::int64_t& value) {
    reader.value_i64(value);
}

inline void read(Reader& reader, std::uint32_t& value) {
    reader.value_u32(value);
}

inline void read(Reader& reader, std::uint64_t& value) {
    reader.value_u64(value);
}

inline void read(Reader& reader, float& value) {
    reader.value_f32(value);
}

inline void read(Reader& reader, double& value) {
    reader.value_f64(value);
}

inline void read(Reader& reader, std::string& value) {
    reader.value_string(value);
}

inline void read(Reader& reader, bool& value) {
    reader.value_bool(value);
}

template <annotated_enum T>
void read(Reader& reader, T& value) {
    value = (T)reader.enumerate(enum_labels<T>());
}

template <readable T>
void read(Reader& reader, std::optional<T>& value) {
    if (reader.optional()) {
        value.emplace();
        reader.value(value.value());
    } else {
        value = std::nullopt;
    }
}

template <annotated_variant T>
void read(Reader& reader, T& value) {
    const char* label = reader.variant(variant_labels<T>());
    value = variant_from_label<T>(label);
    std::visit([&](auto& value) {
        reader.value(value);
    }, value);
}

template <readable T>
void read(Reader& reader, std::vector<T>& value) {
    value.clear();
    reader.list_begin();
    while (reader.list_next()) {
        value.emplace_back();
        reader.value(value.back());
    }
    reader.list_end();
}

template <readable T, std::size_t Size>
void read(Reader& reader, std::array<T, Size>& value) {
    reader.tuple_begin();
    for (auto& element: value) {
        reader.tuple_next();
        reader.value(element);
    }
    reader.tuple_end();
}

template <readable K, readable V>
void read(Reader& reader, std::unordered_map<K, V>& value) {
    if constexpr(std::is_same_v<K, std::string>) {
        std::string key;
        reader.map_begin();
        while (reader.map_next(key)) {
            V element;
            reader.value(element);
            value.emplace(key, element);
        }
        reader.map_end();
    }
    if constexpr(!std::is_same_v<K, std::string>) {
        std::pair<K, V> pair;
        reader.list_begin();
        while (reader.list_next()) {
            reader.tuple_begin();
            reader.value(pair.first);
            reader.value(pair.second);
            reader.tuple_end();
            value.insert(pair);
        }
        reader.list_end();
    }
}

} // namespace datapack
