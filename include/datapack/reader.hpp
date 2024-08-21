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
    Packer(bool trivial_as_binary=false, bool use_constraints=true, bool is_exhaustive=false):
        trivial_as_binary_(trivial_as_binary),
        use_constraints(use_constraints),
        is_exhaustive_(is_exhaustive)
    {}

    template <readable T>
    void value(T& value) {
        if (std::is_trivially_constructible_v<T> && !std::is_arithmetic_v<T> && trivial_as_binary_) {
            auto [data, length] = binary_data(1, sizeof(T));
            value = *(const T*)data;
        } else {
            pack(value, *this);
        }
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

    virtual void variant_begin(const std::span<const char*>& labels) = 0;
    virtual bool variant_match(const char* label) = 0;
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

    void set_error(const char* error) {
        error_ = std::string(error);
    }
    const std::optional<std::string>& error() {
        return error_;
    }

    bool trivial_as_binary() const {
        return trivial_as_binary_;
    }

    bool is_exhaustive() const {
        return is_exhaustive_;
    }

private:
    const bool trivial_as_binary_;
    const bool use_constraints;
    const bool is_exhaustive_;

    std::optional<std::string> error_;
};

} // namespace datapack
