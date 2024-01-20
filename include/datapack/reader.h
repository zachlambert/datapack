#pragma once

#include <concepts>
#include <exception>
#include <string>
#include <memory>

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

class Reader {

};

class Readable {
public:
    virtual void read(Reader& reader) const;
};

template <typename T>
concept readable = requires(T& a, Reader& reader)
{
    { read(reader, a) };
};

template <readable T>
class ReadableImpl: public Readable {
public:
    template <typename... Args>
    ReadableImpl(Args&&... args):
        value(std::make_unique<T>(std::forward(args...)))
    {}
    void read(Reader& reader) const override {
        read<T>(reader, *value);
    }
private:
    std::unique_ptr<T> value;
};

} // namespace datapack
