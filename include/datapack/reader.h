#pragma once

#include <concepts>
#include <exception>
#include <string>
#include <vector>
#include <tsl/ordered_map.h>
#include <optional>
#include "datapack/binary.h"


namespace datapack {

class ReadException: public std::exception {
public:
    ReadException(const std::string& message):
        message(message)
    {}

private:
    const char* what() const noexcept override
    {
        return message.c_str();
    }
    std::string message;
};

class Reader;

template <typename T>
concept readable = requires(T& value, Reader& reader)
{
    { read(reader, value) };
};

// Note: Child class T can return T& instead of Reader& on the overriden
// functions
class Reader {
public:
    virtual Reader& key(const std::string& key) = 0; // When in object
    virtual Reader& next() = 0;                      // When in array

    virtual Reader& i32(int& value) = 0;
    virtual Reader& i64(long& value) = 0;
    virtual Reader& f32(float& value) = 0;
    virtual Reader& f64(double& value) = 0;
    virtual Reader& string(std::string& value) = 0;
    virtual Reader& boolean(bool& value) = 0;
    virtual Reader& binary(const binary_t& value) = 0;

    template <readable T>
    Reader& value(T& value);

    virtual bool peek_null() = 0;
    template <readable T>
    Reader& optional(std::optional<T>& value) {
        if (peek_null()) {
            value = std::nullopt;
        } else {
            value.emplace();
            this->value(value.value());
        }
        return *this;
    }

    virtual Reader& start_array() = 0;
    virtual Reader& end_array() = 0;

    virtual Reader& start_object() = 0;
    virtual Reader& end_object() = 0;
};

template <readable T>
Reader& Reader::value(T& value) {
    read(*this, value);
    return *this;
}

class Readable {
public:
    virtual void read(Reader& reader) = 0;
};

void read(Reader& reader, const int& value);
void read(Reader& reader, const long& value);
void read(Reader& reader, const float& value);
void read(Reader& reader, const double& value);
void read(Reader& reader, const std::string& value);
void read(Reader& reader, const bool& value);
void read(Reader& reader, const std::nullopt_t& value);
void read(Reader& reader, const binary_t& value);
void read(Reader& reader, const Readable& value);

} // namespace datapack
