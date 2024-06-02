#pragma once

#include <concepts>
#include <string>
#include <stdexcept>
#include <vector>
#include <stack>
#include <type_traits>
#include <tuple>
#include "datapack/constraint.hpp"


namespace datapack {

class Reader;

template <typename T>
concept readable = requires(Reader& reader, T& value) {
    { read(reader, value) };
};

template <typename T>
concept readable_binary = requires(Reader& reader, T& value) {
    { read_binary(reader, value) };
};

template <typename T>
concept readable_either = readable_binary<T> || readable<T>;

class Readable {
public:
    virtual void read(Reader&) = 0;
};

inline void read(Reader& reader, Readable& value) {
    value.read(reader);
}

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

class Reader {
public:
    Reader(bool is_binary=false, bool use_constraints=true, bool is_exhaustive=false):
        is_binary(is_binary),
        use_constraints(use_constraints),
        is_exhaustive_(is_exhaustive),
        constraint_(nullptr)
    {}

    template <readable_either T>
    void value(T& value) {
        if constexpr(readable<T> && readable_binary<T>) {
            if (is_binary) {
                read_binary(*this, value);
            } else {
                read(*this, value);
            }
        }
        if constexpr(!readable_binary<T>) {
            read(*this, value);
        }
        if constexpr(!readable<T>) {
            read_binary(*this, value);
        }
    }

    template <readable_either T>
    void value(const char* key, T& value) {
        object_next(key);
        this->value(value);
    }

    template <readable_either T, is_constraint Constraint>
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

    template <readable_either T, is_constraint Constraint>
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

    virtual void value_string(std::string& value) = 0;
    virtual void value_bool(bool& value) = 0;

    virtual int enumerate(const std::vector<const char*>& labels) = 0;
    virtual bool optional() = 0;
    virtual void variant_begin(const std::vector<const char*>& labels) = 0;
    virtual bool variant_match(const char* label) = 0;
    virtual void variant_end() = 0;

    virtual std::tuple<const std::uint8_t*, std::size_t> binary_data() = 0;

    // BinaryReader and BinarySchemaBuilder only
    virtual std::size_t binary_begin(std::size_t stride) {
        error("Not supported");
        return 0;
    }
    virtual void binary_end() {
        error ("Not supported");
    }

    virtual void object_begin() = 0;
    virtual void object_end() = 0;
    virtual void object_next(const char* key) = 0;

    virtual void tuple_begin() = 0;
    virtual void tuple_end() = 0;
    virtual void tuple_next() = 0;

    virtual void map_begin() = 0;
    virtual void map_end() = 0;
    virtual bool map_next(std::string& key) = 0;

    virtual void list_begin() = 0;
    virtual void list_end() = 0;
    virtual bool list_next() = 0;

    void error(const std::string& error) {
        throw ReadException(error);
    }

    bool is_exhaustive() const {
        return is_exhaustive_;
    }

    template <typename T>
    const T* constraint() const {
        return dynamic_cast<const T*>(constraint_);
    }

private:
    const bool is_binary;
    const bool use_constraints;
    const bool is_exhaustive_;
    const ConstraintBase* constraint_;
};

} // namespace datapack
