#pragma once

#include <concepts>
#include <exception>
#include <string>
#include <memory>
#include <vector>
#include "datapack/binary.h"


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

class Writeable;

class Writer {
public:
    virtual void key(const std::string& key) = 0; // When in object
    virtual void next() = 0;                      // When in array

    virtual void i32(int value) = 0;
    virtual void i64(long value) = 0;
    virtual void f32(float value) = 0;
    virtual void f64(double value) = 0;
    virtual void string(const std::string& value) = 0;
    virtual void boolean(bool value) = 0;
    virtual void binary(const binary_t& value) = 0;

    virtual void start_array() = 0;
    virtual void end_array() = 0;

    virtual void start_object() = 0;
    virtual void end_object() = 0;
};

class Writeable {
public:
    virtual void write(Writer& writer) const;
};

template <typename T>
concept writeable = requires(const T& value, Writer& writer)
{
    { write(writer, value) };
};

template <writeable T>
class WriteableImpl: public Writeable {
public:
    template <typename... Args>
    WriteableImpl(Args&&... args):
        value(std::make_unique<T>(std::forward(args...)))
    {}
    void write(Writer& writer) const override {
        write<T>(writer, *value);
    }
private:
    std::unique_ptr<T> value;
};

} // namespace datapack
