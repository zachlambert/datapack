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
concept writeable_class = requires(Writer& writer, const T& value) {
    { value.write(writer) };
};

template <writeable_class T>
inline void write(Writer& writer, const T& value) {
    value.write(writer);
}

class Writer {
public:
    Writer(bool use_binary_arrays = false):
        use_binary_arrays_(use_binary_arrays)
    {}

    template <writeable T>
    void value(const T& value) {
        write(*this, value);
    }

    template <writeable T>
    void value(const char* key, const T& value) {
        object_next(key);
        this->value(value);
    }

    template <writeable T, is_constraint Constraint>
    void value(T& value, const Constraint&) {
        // Ignore constraint
        this->value(value);
    }

    template <writeable T, is_constraint Constraint>
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

    virtual void binary_data(const std::uint8_t* data, std::size_t size) = 0;

    virtual void object_begin() = 0;
    virtual void object_end() = 0;
    virtual void object_next(const char* key) = 0;

    virtual void tuple_begin() = 0;
    virtual void tuple_end() = 0;
    virtual void tuple_next() = 0;

    virtual void map_begin() = 0;
    virtual void map_end() = 0;
    virtual void map_next(const std::string& key) = 0;

    virtual void list_begin(bool is_array = false) = 0;
    virtual void list_end() = 0;
    virtual void list_next() = 0;

    bool use_binary_arrays() const {
        return use_binary_arrays_;
    }

private:
    const bool use_binary_arrays_;
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
