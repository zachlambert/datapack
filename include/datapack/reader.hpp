#pragma once

#include <concepts>
#include <exception>
#include <string>
#include <vector>
#include <tsl/ordered_map.h>
#include <optional>
#include "datapack/binary.hpp"


namespace datapack {

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

class Reader;

template <typename T>
concept readable = requires(T& value, Reader& reader) {
    { read(reader, value) };
};

// Note: Child class T can return T& instead of Reader& on the overriden
// functions
class Reader {
public:
    virtual void i32(int& value) = 0;
    virtual void i64(long& value) = 0;
    virtual void f32(float& value) = 0;
    virtual void f64(double& value) = 0;
    virtual void string(std::string& value) = 0;
    virtual void boolean(bool& value) = 0;
    virtual bool null() = 0;
    virtual void binary(binary_t& value) = 0;

    virtual void object_begin() = 0;
    virtual void object_end() = 0;
    virtual void object_element(const std::string& key) = 0;

    virtual void array_begin() = 0;
    virtual void array_end() = 0;
    virtual bool array_element() = 0;

    template <readable T>
    void value(T& value);
};

template <readable T>
void Reader::value(T& value) {
    read(*this, value);
}

class Readable {
public:
    virtual void read(Reader& reader) = 0;
};

void read(Reader& reader, int& value);
void read(Reader& reader, long& value);
void read(Reader& reader, float& value);
void read(Reader& reader, double& value);
void read(Reader& reader, std::string& value);
void read(Reader& reader, bool& value);
void read(Reader& reader, binary_t& value);
void read(Reader& reader, Readable& value);

template <readable T>
void read(Reader& reader, std::optional<T>& value) {
    if (reader.null()) {
        value = std::nullopt;
    } else {
        value.emplace();
        reader.value(value.value());
    }
}

template <readable T>
void read(Reader& reader, std::vector<T>& value) {
    value.clear();
    reader.array_begin();
    while (reader.array_element()) {
        value.emplace_back();
        reader.value(value.back());
    }
    reader.array_end();
}

} // namespace datapack
