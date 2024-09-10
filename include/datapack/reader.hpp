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
class Packer<MODE_READ> {
public:
    Packer(bool trivial_as_binary=false):
        trivial_as_binary_(trivial_as_binary)
    {}

    template <readable T>
    void value(T& value) {
        pack(value, *this);
    }

    template <readable T>
    void value(const char* key, T& value) {
        object_next(key);
        this->value(value);
    }

    virtual void primitive(Primitive primitive, void* value) = 0;
    virtual const char* string() = 0;
    virtual int enumerate(const std::span<const char*>& labels) = 0;

    virtual bool optional_begin() = 0;
    virtual void optional_end() = 0;

    virtual int variant_begin(const std::span<const char*>& labels) = 0;
    virtual void variant_end() = 0;

    virtual std::tuple<const std::uint8_t*, std::size_t>
        binary_data(std::size_t length, std::size_t stride) = 0;

    virtual void object_begin(std::size_t size = 0) = 0;
    virtual void object_end(std::size_t size = 0) = 0;
    virtual void object_next(const char* key) = 0;

    virtual void tuple_begin(std::size_t size = 0) = 0;
    virtual void tuple_end(std::size_t size = 0) = 0;
    virtual void tuple_next() = 0;

    virtual void list_begin(bool is_trivial = false) = 0;
    virtual void list_end() = 0;
    virtual bool list_next() = 0;

    virtual void map_begin() = 0;
    virtual bool map_key() = 0;
    virtual void map_value() = 0;
    virtual bool map_end() = 0;

    void invalidate() { valid_ = false; }
    bool valid() const { return valid_; }
    bool trivial_as_binary() const { return trivial_as_binary_; }

private:
    bool valid_;
    const bool trivial_as_binary_;
};

} // namespace datapack
