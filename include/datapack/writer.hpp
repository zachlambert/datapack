#pragma once

#include <concepts>
#include <string>
#include <vector>
#include "datapack/constraint.hpp"


namespace datapack {

class Writer;

template <typename T>
concept writeable = requires(Writer& writer, const T& value) {
    { write(writer, value) };
};

template <typename T>
concept writeable_binary = requires(Writer& writer, const T& value) {
    { write_binary(writer, value) };
};

template <typename T>
concept writeable_either = writeable<T> || writeable_binary<T>;

class Writeable {
public:
    virtual void write(Writer&) const = 0;
};

inline void write(Writer& writer, const Writeable& value) {
    value.write(writer);
}


class Writer {
public:
    Writer(bool is_binary = false):
        is_binary(is_binary),
        is_writeable_binary_(false)
    {}

    template <writeable_either T>
    void value(const T& value) {
        if constexpr(writeable<T> && writeable_binary<T>) {
            is_writeable_binary_ = true;
            if (is_binary) {
                write_binary(*this, value);
            } else {
                write(*this, value);
            }
        }
        if constexpr(!writeable_binary<T>) {
            write(*this, value);
        }
        if constexpr(!writeable<T>) {
            is_writeable_binary_ = true;
            write_binary(*this, value);
        }
        is_writeable_binary_ = false;
    }

    template <writeable_either T>
    void value(const char* key, const T& value) {
        object_next(key);
        this->value(value);
    }

    template <writeable_either T, is_constraint Constraint>
    void value(T& value, const Constraint&) {
        // Ignore constraint
        this->value(value);
    }

    template <writeable_either T, is_constraint Constraint>
    void value(const char* key, const T& value, const Constraint&) {
        object_next(key);
        // Ignore constraint
        this->value(value);
    }

    virtual void value_i32(std::int32_t value) = 0;
    virtual void value_i64(std::int64_t value) = 0;
    virtual void value_u32(std::uint32_t value) = 0;
    virtual void value_u64(std::uint64_t value) = 0;

    virtual void value_f32(float value) = 0;
    virtual void value_f64(double value) = 0;

    virtual void value_string(const std::string&) = 0;
    virtual void value_bool(bool value) = 0;

    virtual void enumerate(int value, const std::vector<const char*>& labels) = 0;
    virtual void optional(bool has_value) = 0;
    virtual void variant_begin(const char* label, const std::vector<const char*>& labels) = 0;
    virtual void variant_end() = 0;

    virtual void binary(std::size_t size, const std::uint8_t* data, std::size_t stride = 0) = 0;

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

    bool is_writeable_binary() const {
        return is_writeable_binary_;
    }

private:
    const bool is_binary;
    bool is_writeable_binary_;
};

class WriteException: public std::exception {
public:
    WriteException(const std::string& message):
        message(message)
    {}

private:
    const char* what() const noexcept override {
        return message.c_str();
    }
    std::string message;
};

} // namespace datapack
