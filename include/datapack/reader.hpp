#pragma once

#include <concepts>
#include <string>
#include <stdexcept>
#include <vector>


namespace datapack {

class Reader;

template <typename T>
concept readable = requires(Reader& reader, T& value) {
    { read(reader, value) };
};

template <typename T>
concept readable_binary = requires(Reader& reader, T& value, std::size_t expected_size) {
    { read_binary(reader, value, expected_size) };
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
        read(*this, value);
    }

    template <readable_binary T>
    void value_binary(T& value, std::size_t expected_size = 0) {
        read_binary(*this, value, expected_size);
    }

    template <readable_binary T>
    void value_binary(const char* key, T& value, std::size_t expected_size = 0) {
        object_next(key);
        read_binary(*this, value, expected_size);
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
    virtual const char* variant_begin(const std::vector<const char*>& labels) = 0;
    virtual void variant_end() = 0;

    virtual std::size_t binary_size(std::size_t expected_size=0) = 0;
    virtual void binary_data(std::uint8_t* data) = 0;

    virtual void object_begin() = 0;
    virtual void object_end() = 0;
    virtual void object_next(const char* key) = 0;

    virtual void tuple_begin() = 0;
    virtual void tuple_end() = 0;
    virtual void tuple_next() = 0;

    virtual void map_begin() = 0;
    virtual void map_end() = 0;
    virtual bool map_next(std::string& key) = 0;

    virtual void list_begin() = 0;
    virtual void list_end() = 0;
    virtual bool list_next() = 0;

    void error(const std::string& error) {
        throw std::runtime_error(error);
    }
};

class ReadException: public std::exception {
public:
    ReadException(const std::string& message):
        message(message)
    {}

private:
    const char* what() const noexcept override {
        return message.c_str();
    }
    std::string message;
};

} // namespace datapack
