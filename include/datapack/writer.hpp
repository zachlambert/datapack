#pragma once

#include <string>
#include <span>
#include <vector>
#include <optional>
#include <tuple>
#include "datapack/packer.hpp"
#include "datapack/primitive.hpp"


namespace datapack {

template <>
class Packer<MODE_WRITE> {
public:
    Packer(bool trivial_as_binary = false):
        trivial_as_binary_(trivial_as_binary)
    {}

    template <writeable T>
    void value(const T& value) {
        pack(value, *this);
    }

    template <writeable T>
    void value(const char* key, const T& value) {
        object_next(key);
        this->value(value);
    }

    virtual void primitive(Primitive primitive, const void* value) = 0;
    virtual void string(const char* string) = 0;
    virtual void enumerate(int value, const char* label) = 0;

    virtual void optional_begin(bool has_value) = 0;
    virtual void optional_end() = 0;

    virtual void variant_begin(int value, const char* label) = 0;
    virtual void variant_end() = 0;

    virtual void binary_data(const std::uint8_t* data, std::size_t length, std::size_t stride, bool fixed_length) = 0;

    virtual void object_begin(std::size_t size = 0) = 0;
    virtual void object_end(std::size_t size = 0) = 0;
    virtual void object_next(const char* key) = 0;

    virtual void tuple_begin(std::size_t size = 0) = 0;
    virtual void tuple_end(std::size_t size = 0) = 0;
    virtual void tuple_next() = 0;

    virtual void list_begin(bool is_trivial = false) = 0;
    virtual void list_end() = 0;
    virtual void list_next() = 0;

    bool trivial_as_binary() const {
        return trivial_as_binary_;
    }

private:
    const bool trivial_as_binary_;
};

} // namespace datapack
