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
    virtual int i32() = 0;
    virtual long i64() = 0;
    virtual float f32() = 0;
    virtual double f64() = 0;
    virtual std::string string() = 0;
    virtual bool boolean() = 0;
    virtual bool null() = 0;
    virtual binary_t binary() = 0;

    virtual void object_begin() = 0;
    virtual void object_end() = 0;
    virtual void object_element(const std::string& key) = 0;

    virtual void array_begin() = 0;
    virtual void array_end() = 0;
    virtual bool array_element() = 0;

    template <readable T>
    T value();
    template <readable T>
    T value_like(const T& _dummy);
};

template <readable T>
T Reader::value() {
    T value;
    read(*this, value);
    return value;
}

template <readable T>
T Reader::value_like(const T& _dummy) {
    T value;
    read(*this, value);
    return value;
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
        value.push_back(reader.value<T>());
    }
    reader.array_end();
}

} // namespace datapack
