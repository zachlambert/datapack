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

enum class ContainerAction {
    None,
    Push,
    Pop
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
    virtual void variant_tokenize(int index) {}; // Only used by tokenizer
    virtual void variant_end() = 0;

    virtual void binary_data(
        std::uint8_t* data,
        std::size_t length,
        std::size_t stride,
        bool fixed_length) = 0;

    virtual void object_begin(std::size_t=0) = 0;
    virtual void object_end(std::size_t=0) = 0;
    virtual void object_next(const char* key) = 0;

    virtual void tuple_begin(std::size_t=0) = 0;
    virtual void tuple_end(std::size_t=0) = 0;
    virtual void tuple_next() = 0;

    virtual void list_begin(bool=false) = 0;
    virtual void list_next() = 0;
    virtual ContainerAction list_end() = 0;

    virtual void map_begin() = 0;
    virtual void map_key() = 0;
    virtual void map_value() = 0;
    virtual void map_end() = 0;
    virtual bool map_action_begin() = 0;
    virtual ContainerAction map_action_end() = 0;

    void invalidate() { valid_ = false; }
    bool valid() const { return valid_; }
    bool is_tokenizer() const { return is_tokenizer_; }

    template <typename Constraint>
    requires std::is_base_of_v<ConstraintBase, Constraint>
    const Constraint* constraint() const {
        return dynamic_cast<const Constraint*>(constraint_);
    }

private:
    bool valid_;
    const bool is_tokenizer_;
    ConstraintBase* constraint_;
};

} // namespace datapack
