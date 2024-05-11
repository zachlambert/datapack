#pragma once

#include <type_traits>
#include <optional>
#include <concepts>
#include <string>
#include <variant>
#include <array>
#include <stdexcept>
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

class Reader {
public:
    template <readable T>
    void value(T& value) {
        read(*this, value);
    }

    void value(Readable& value) {
        value.read(*this);
    }

    virtual void value_i32(std::int32_t& value) = 0;
    virtual void value_i64(std::int64_t& value) = 0;
    virtual void value_u32(std::uint32_t& value) = 0;
    virtual void value_u64(std::uint64_t& value) = 0;

    virtual void value_f32(float& value) = 0;
    virtual void value_f64(double& value) = 0;

    virtual void value_string(std::string& value) = 0;
    virtual void value_bool(bool& value) = 0;

    virtual void enumerate(int& value, const std::vector<std::string>& labels) = 0;

    template <annotated_enum T>
    void value(T& value) {
        value = (T)enumerate(variant_labels<T>());
    }

    virtual bool optional() = 0;

    template <readable T>
    void value(std::optional<T>& value) {
        if (optional()) {
            value.emplace();
            this->value(value.value());
        } else {
            value = std::nullopt;
        }
    }

    virtual const char* variant(const std::vector<std::string>& labels) = 0;

    template <annotated_variant T>
    void value(T& value) {
        const char* label = variant(variant_labels<T>());
        value = variant_from_label<T>(label);
        std::visit([&](auto& value) {
            this->value(value);
        }, value);
    }

    virtual std::size_t binary_size() = 0;
    virtual void binary_data(std::uint8_t* data) = 0;

    template <typename T>
    void value_binary(std::vector<T>& value) {
        static_assert(std::is_pod_v<T>);
        std::size_t size = binary_size();
        if (size % sizeof(T) != 0) {
            error("Incorrect binary size");
        }
        value.resize(size / sizeof(T));
        binary_data((std::uint8_t*)value.data());
    }

    template <typename T, std::size_t N>
    void value_binary(std::array<T, N>& value) {
        static_assert(std::is_pod_v<T>);
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

} // namespace datapack
