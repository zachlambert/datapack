#pragma once

#include <array>
#include <concepts>
#include <optional>
#include <string>
#include <type_traits>
#include <variant>
#include "datapack/enum.hpp"
#include "datapack/variant.hpp"


namespace datapack {

class Writer;

template <typename T>
concept writeable = requires(Writer& writer, const T& value) {
    { write(writer, value) };
};

class Writeable {
public:
    virtual void write(Writer&) const = 0;
};

class Writer {
public:
    template <writeable T>
    void value(T& value) {
        write(*this, value);
    }

    void value(const Writeable& value) {
        value.write(*this);
    }

    virtual void value_i32(std::int32_t value) = 0;
    virtual void value_i64(std::int64_t value) = 0;
    virtual void value_u32(std::uint32_t value) = 0;
    virtual void value_u64(std::uint64_t value) = 0;

    virtual void value_f32(float value) = 0;
    virtual void value_f64(double value) = 0;

    virtual void value_string(const std::string&) = 0;
    virtual void value_bool(bool value) = 0;

    virtual void enumerate(int value, const std::vector<std::string>& labels) = 0;

    template <annotated_enum T>
    void value(const T& value) {
        enumerate((int)value, variant_labels<T>());
    }

    virtual void optional(bool has_value);

    template <writeable T>
    void value(const std::optional<T>& value) {
        optional(value.has_value());
        if (value.has_value()) {
            this->value(value.value());
        }
    }

    virtual void variant(const char* label, const std::vector<std::string>& labels) = 0;

    template <annotated_variant T>
    void value(const T& value) {
        variant(variant_to_label(value), variant_labels<T>());
        std::visit([&](const auto& value){
            this->value(value);
        }, value);
    }

    virtual void binary(std::size_t size, const std::uint8_t* data) = 0;

    template <typename T>
    void value_binary(const std::vector<T>& value) {
        static_assert(std::is_pod_v<T>);
        binary(value.size() * sizeof(T), value.data());
    }

    template <typename T, std::size_t N>
    void value_binary(std::array<T, N>& value) {
        static_assert(std::is_pod_v<T>);
        binary(value.size() * sizeof(T), value.data());
    }

    virtual void object_begin() = 0;
    virtual void object_end() = 0;
    virtual void object_next(const char* key) = 0;

    virtual void tuple_begin() = 0;
    virtual void tuple_end() = 0;
    virtual void tuple_next() = 0;

    virtual void map_begin() = 0;
    virtual void map_end() = 0;
    virtual void map_next(const std::string& key) = 0;

    virtual void list_begin() = 0;
    virtual void list_end() = 0;
    virtual void list_next() = 0;
};

inline void write(Writer& writer, std::int32_t value) {
    writer.value_i32(value);
}

inline void write(Writer& writer, std::int64_t value) {
    writer.value_i64(value);
}

inline void write(Writer& writer, std::uint32_t value) {
    writer.value_u32(value);
}

inline void write(Writer& writer, std::uint64_t value) {
    writer.value_u64(value);
}

inline void write(Writer& writer, float value) {
    writer.value_f32(value);
}

inline void write(Writer& writer, double value) {
    writer.value_f64(value);
}

inline void write(Writer& writer, std::string value) {
    writer.value_string(value);
}

inline void write(Writer& writer, bool value) {
    writer.value_bool(value);
}

} // namespace datapack
