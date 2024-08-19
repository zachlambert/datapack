#pragma once

#include <concepts>
#include <type_traits>
#include <tuple>
#include <span>
#include <optional>
#include <string>
#include "datapack/constraint.hpp"


namespace datapack {

class Reader;

template <typename T>
concept readable = requires(Reader& reader, T& value) {
    { read(reader, value) };
};

template <typename T>
concept readable_class = requires(Reader& reader, T& value) {
    { value.read(reader) };
};

template <readable_class T>
inline void read(Reader& reader, T& value) {
    value.read(reader);
}

enum class ListNext {
    Next,
    Remove,
    Insert,
    End
};

class Reader {
public:
    Reader(bool trivial_as_binary=false, bool use_constraints=true, bool is_exhaustive=false):
        trivial_as_binary_(trivial_as_binary),
        use_constraints(use_constraints),
        is_exhaustive_(is_exhaustive),
        constraint_(nullptr)
    {}

    template <readable T>
    void value(T& value) {
        if (std::is_trivially_constructible_v<T> && !std::is_arithmetic_v<T> && trivial_as_binary_) {
            auto [data, length] = binary_data(1, sizeof(T));
            value = *(const T*)data;
        } else {
            read(*this, value);
        }
    }

    template <readable T>
    void value(const char* key, T& value) {
        object_next(key);
        this->value(value);
    }

    template <readable T, is_constraint Constraint>
    requires is_constrained<T, Constraint>
    void value(T& value, const Constraint& constraint) {
        if (use_constraints) {
            constraint_ = &constraint;
        }
        this->value(value);
        if (use_constraints) {
            if (!validate(value, constraint)) {
                set_error("Constraint failed");
            }
            constraint_ = nullptr;
        }
    }

    template <readable T, is_constraint Constraint>
    requires is_constrained<T, Constraint>
    void value(const char* key, T& value, const Constraint& constraint) {
        object_next(key);
        this->value(value, constraint);
    }

    virtual void value_i32(std::int32_t& value) = 0;
    virtual void value_i64(std::int64_t& value) = 0;
    virtual void value_u32(std::uint32_t& value) = 0;
    virtual void value_u64(std::uint64_t& value) = 0;

    virtual void value_f32(float& value) = 0;
    virtual void value_f64(double& value) = 0;

    virtual const char* value_string(const char* current) = 0;
    virtual void value_bool(bool& value) = 0;

    virtual int enumerate(const std::span<const char*>& labels) = 0;

    virtual bool optional_begin(bool current_has_value) = 0;
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
    virtual ListNext list_next(bool has_next) = 0;

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

    template <typename T>
    const T* constraint() const {
        return dynamic_cast<const T*>(constraint_);
    }

private:
    const bool trivial_as_binary_;
    const bool use_constraints;
    const bool is_exhaustive_;
    const ConstraintBase* constraint_;

    std::optional<std::string> error_;
};

} // namespace datapack
