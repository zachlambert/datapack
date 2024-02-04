#pragma once

#include <concepts>
#include <exception>
#include <string>
#include <vector>
#include <optional>
#include <type_traits>
#include "datapack/binary.hpp"


namespace datapack {

class WriteException: public std::exception {
public:
    WriteException(const std::string& message):
        message(message)
    {}

private:
    const char* what() const noexcept override
    {
        return message.c_str();
    }
    std::string message;
};

class Writer;

template <typename T>
concept writeable = requires(const T& value, Writer& writer)
{
    { write(writer, value) };
};

class Writer {
public:
    virtual void i32(int value) = 0;
    virtual void i64(long value) = 0;
    virtual void f32(float value) = 0;
    virtual void f64(double value) = 0;
    virtual void string(const std::string& value) = 0;
    virtual void boolean(bool value) = 0;
    virtual void null() = 0;
    virtual void binary(const binary_t& value) = 0;

    virtual void object_begin() = 0;
    virtual void object_end() = 0;
    virtual void object_element(const std::string& key) = 0;

    virtual void array_begin() = 0;
    virtual void array_end() = 0;
    virtual void array_element() = 0;

    template <writeable T>
    void value(const T& value);
};

template <writeable T>
void Writer::value(const T& value) {
    write(*this, value);
}

class Writeable {
public:
    virtual void write(Writer& writer) const = 0;
};

void write(Writer& writer, const int& value);
void write(Writer& writer, const long& value);
void write(Writer& writer, const float& value);
void write(Writer& writer, const double& value);
void write(Writer& writer, const std::string& value);
void write(Writer& writer, const bool& value);
void write(Writer& writer, const std::nullopt_t& value);
void write(Writer& writer, const binary_t& value);
void write(Writer& writer, const Writeable& value);

template <writeable T>
void write(Writer& writer, const std::optional<T>& value){
    if (value.has_value()) {
        write(writer, value.value());
    } else {
        writer.null();
    }
}

template <writeable T>
void write(Writer& writer, const std::vector<T>& value) {
    writer.array_begin();
    for (const auto& element: value) {
        writer.array_element();
        writer.value(element);
    }
    writer.array_end();
}

} // namespace datapack
