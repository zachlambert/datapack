#pragma once

#include <concepts>
#include <string>
#include <vector>


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

class Writeable {
public:
    virtual void write(Writer&) const = 0;
};

inline void write(Writer& writer, const Writeable& value) {
    value.write(writer);
}

class Writer {
public:
    template <writeable T>
    void value(T& value) {
        write(*this, value);
    }

    template <writeable T>
    void value(const char* key, T& value) {
        object_next(key);
        write(*this, value);
    }

    template <writeable_binary T>
    void value_binary(const T& value) {
        write_binary(*this, value);
    }

    template <writeable_binary T>
    void value_binary(const char* key, const T& value) {
        object_next(key);
        write_binary(*this, value);
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

    virtual void binary(std::size_t size, const std::uint8_t* data) = 0;

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
