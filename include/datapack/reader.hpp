#pragma once

#include <concepts>
#include <type_traits>
#include <tuple>
#include <span>
#include "datapack/constraint.hpp"

#ifndef EMBEDDED
#include <stdexcept>
#endif


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

#ifndef EMBEDDED
class ReadException: public std::exception {
public:
    ReadException(const std::string& message):
        message(message)
    {}

private:
    const char* what() const noexcept override {
        return message.c_str();
    }
    std::string message;
};
#endif

class Reader {
public:
    Reader(bool use_binary_arrays=false, bool use_constraints=true, bool is_exhaustive=false):
        use_binary_arrays_(use_binary_arrays),
        use_constraints(use_constraints),
        is_exhaustive_(is_exhaustive),
        constraint_(nullptr),
        has_error_(false)
    {}

    template <readable T>
    void value(T& value) {
        read(*this, value);
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
                error("Constraint failed");
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

    virtual const char* value_string() = 0;
    virtual void value_bool(bool& value) = 0;

    virtual int enumerate(const std::span<const char*>& labels) = 0;

    virtual bool optional_begin() = 0;
    virtual void optional_end() = 0;

    virtual void variant_begin(const std::span<const char*>& labels) = 0;
    virtual bool variant_match(const char* label) = 0;
    virtual void variant_end() = 0;

    virtual std::tuple<const std::uint8_t*, std::size_t> binary_data() = 0;

    virtual void object_begin() = 0;
    virtual void object_end() = 0;
    virtual void object_next(const char* key) = 0;

    virtual void tuple_begin() = 0;
    virtual void tuple_end() = 0;
    virtual void tuple_next() = 0;

    virtual void list_begin(bool is_array = false) = 0;
    virtual void list_end() = 0;
    virtual bool list_next() = 0;

    void error(const char* error) {
        has_error_ = true;
#ifndef EMBEDDED
        throw ReadException(error);
#endif
    }
    bool has_error() const {
        return has_error_;
    }

    bool use_binary_arrays() const {
        return use_binary_arrays_;
    }

    bool is_exhaustive() const {
        return is_exhaustive_;
    }

    template <typename T>
    const T* constraint() const {
        return dynamic_cast<const T*>(constraint_);
    }

private:
    const bool use_binary_arrays_;
    const bool use_constraints;
    const bool is_exhaustive_;
    const ConstraintBase* constraint_;
    bool has_error_;
};

} // namespace datapack
