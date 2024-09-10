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
    Packer(bool is_tokenizer = false):
        valid_(true),
        is_tokenizer_(is_tokenizer),
        constraint_(nullptr)
    {}

    template <editable T>
    void value(T& value) {
        pack(value, *this);
    }

    template <editable T>
    void value(const char* key, T& value) {
        object_next(key);
        this->value(value);
    }

    template <editable T, typename Constraint>
    requires is_constrained<T, Constraint>
    void value(T& value, const Constraint& constraint) {
        constraint_ = &constraint;
        pack(value, *this);
        if (!is_tokenizer_ && !constraint.validate(value)) {
            valid_ = false;
        }
        constraint_ = nullptr;
    }

    template <editable T, typename Constraint>
    requires is_constrained<T, Constraint>
    void value(const char* key, T& value, const Constraint& constraint) {
        object_next(key);
        this->value(value, constraint);
    }

    virtual void primitive(Primitive primitive, void* value) = 0;
    virtual const char* string(const char* value) = 0;
    virtual int enumerate(int value, const std::span<const char*>& labels) = 0;

    virtual bool optional_begin(bool has_value) = 0;
    virtual void optional_end() = 0;

    virtual int variant_begin(int value, const std::span<const char*>& labels) = 0;
    virtual bool variant_next(int index) = 0;
    virtual void variant_end() = 0;

    virtual std::tuple<const std::uint8_t*, std::size_t>
        binary_data(const std::uint8_t* data, std::size_t length, std::size_t stride) = 0;

    virtual void object_begin() = 0;
    virtual void object_end() = 0;
    virtual void object_next(const char* key) = 0;

    virtual void tuple_begin() = 0;
    virtual void tuple_end() = 0;
    virtual void tuple_next() = 0;

    virtual void list_begin() = 0;
    virtual void list_end() = 0;
    virtual bool list_next(bool has_next, ListAction& action) = 0;

    virtual void map_begin() = 0;
    virtual void map_key() = 0;
    virtual void map_value() = 0;
    virtual void map_end() = 0;
    virtual void map_insert_begin() = 0;
    virtual bool map_insert_end() = 0;

    void invalidate() { valid_ = false; }
    bool valid() const { return valid_; }
    bool is_tokenizer() const { return is_tokenizer_; }
    const ConstraintBase* constraint() const { return constraint_; }

private:
    bool valid_;
    const bool is_tokenizer_;
    ConstraintBase* constraint_;
};

} // namespace datapack
