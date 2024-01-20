#pragma once

#include <concepts>
#include <exception>
#include <string>
#include <vector>
#include <tsl/ordered_map.h>
#include <optional>
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

class Writer;

template <typename T>
concept writeable = requires(const T& value, Writer& writer)
{
    { write(writer, value) };
};

class Writer {
public:
    virtual Writer& key(const std::string& key) = 0; // When in object
    virtual Writer& next() = 0;                      // When in array

    virtual Writer& i32(int value) = 0;
    virtual Writer& i64(long value) = 0;
    virtual Writer& f32(float value) = 0;
    virtual Writer& f64(double value) = 0;
    virtual Writer& string(const std::string& value) = 0;
    virtual Writer& boolean(bool value) = 0;
    virtual Writer& null() = 0;
    virtual Writer& binary(const binary_t& value) = 0;

    template <writeable T>
    Writer& value(const T& value) {
        write(*this, value);
        return *this;
    }

    virtual Writer& start_array() = 0;
    virtual Writer& end_array() = 0;

    virtual Writer& start_object() = 0;
    virtual Writer& end_object() = 0;
};

// TODO: Put in source

static void write(Writer& writer, const int& value) {
    writer.i32(value);
}

static void write(Writer& writer, const long& value) {
    writer.i64(value);
}

static void write(Writer& writer, const float& value) {
    writer.f32(value);
}

static void write(Writer& writer, const double& value) {
    writer.f64(value);
}

static void write(Writer& writer, const std::string& value) {
    writer.string(value);
}

static void write(Writer& writer, const bool& value) {
    writer.boolean(value);
}

static void write(Writer& writer, const std::nullopt_t& value) {
    writer.null();
}

static void write(Writer& writer, const binary_t& value) {
    writer.binary(value);
}

template <typename T>
static void write(Writer& writer, const T& value) {
    static_assert("Not implemented");
}

class Writeable {
public:
    virtual void write(Writer& writer) const = 0;
};

static void write(Writer& writer, const Writeable& value) {
    value.write(writer);
}

template <writeable T>
void write(Writer& writer, std::vector<T>& value) {
    writer.start_array();
    for (const auto& element: value) {
        writer.next();
        writer.value(element);
    }
    writer.end_array();
}

template <writeable T>
void write(Writer& writer, tsl::ordered_map<std::string, T>& value) {
    writer.start_object();
    for (const auto& pair: value) {
        writer.key(pair.first);
        writer.value(pair.second);
    }
    writer.end_object();
}

} // namespace datapack
