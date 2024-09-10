#pragma once

#include <string>
#include <span>
#include <vector>
#include <optional>
#include <tuple>
#include "datapack/packer.hpp"
#include "datapack/primitive.hpp"
#include "datapack/constraint.hpp"


namespace datapack {

enum class ListAction {
    Next,
    Remove,
    Insert
};

template <>
class Packer<MODE_EDIT> {
public:
    Packer()
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
    virtual int enumerate(int value, const std::span<const char*>& labels) = 0;

    virtual bool optional_begin(bool has_value) = 0;
    virtual void optional_end() = 0;

    virtual void variant_begin(const char* value, const std::span<const char*>& labels) = 0;
    virtual bool variant_match(const char* label) = 0;
    virtual void variant_end() = 0;

    virtual std::tuple<const std::uint8_t*, std::size_t>
        binary_data(const std::uint8_t* data, std::size_t length, std::size_t stride) = 0;

    virtual void object_begin(std::size_t size = 0) = 0;
    virtual void object_end(std::size_t size = 0) = 0;
    virtual void object_next(const char* key) = 0;

    virtual void tuple_begin(std::size_t size = 0) = 0;
    virtual void tuple_end(std::size_t size = 0) = 0;
    virtual void tuple_next() = 0;

    virtual void list_begin() = 0;
    virtual void list_end() = 0;
    virtual bool list_next(ListAction& action) = 0;
};

} // namespace datapack
