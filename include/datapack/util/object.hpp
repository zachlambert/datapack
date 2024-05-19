#pragma once

#include <stack>
#include <cstring>
#include "datapack/object.hpp"
#include "datapack/reader.hpp"
#include "datapack/writer.hpp"


namespace datapack {

class ObjectWriter: public Writer {
public:
    ObjectWriter(Object& object);

    void value_i32(std::int32_t value) override;
    void value_i64(std::int64_t value) override;
    void value_u32(std::uint32_t value) override;
    void value_u64(std::uint64_t value) override;

    void value_f32(float value) override;
    void value_f64(double value) override;

    void value_string(const std::string& value) override;
    void value_bool(bool value) override;

    void enumerate(int value, const std::vector<const char*>& labels) override;
    void optional(bool has_value) override;
    void variant_begin(const char* label, const std::vector<const char*>& labels) override;
    void variant_end() override;

    void binary(std::size_t size, const std::uint8_t* data) override;

    void object_begin() override;
    void object_end() override;
    void object_next(const char* key) override;

    void tuple_begin() override;
    void tuple_end() override;
    void tuple_next() override;

    void map_begin() override;
    void map_end() override;
    void map_next(const std::string& key) override;

    void list_begin() override;
    void list_end() override;
    void list_next() override;

private:
    void set_value(const Object::value_t& value);

    Object& object;
    std::stack<Object> nodes;
    std::string next_key;
};


class ObjectReader: public Reader {
public:
    ObjectReader(ConstObject object);

    void value_i32(std::int32_t& value) override;
    void value_i64(std::int64_t& value) override;
    void value_u32(std::uint32_t& value) override;
    void value_u64(std::uint64_t& value) override;

    void value_f32(float& value) override;
    void value_f64(double& value) override;

    void value_string(std::string& value) override;
    void value_bool(bool& value) override;

    int enumerate(const std::vector<const char*>& labels) override;
    bool optional() override;
    const char* variant_begin(const std::vector<const char*>& labels) override;
    void variant_end() override;

    std::size_t binary_size(std::size_t expected_size=0) override;
    void binary_data(std::uint8_t* data) override;

    void object_begin() override;
    void object_end() override;
    void object_next(const char* key) override;

    void tuple_begin() override;
    void tuple_end() override;
    void tuple_next() override;

    void map_begin() override;
    void map_end() override;
    bool map_next(std::string& key) override;

    void list_begin() override;
    void list_end() override;
    bool list_next() override;

private:
    template <typename T>
    bool value_obj_int(T& value) {
        if (auto x = node.get_if<Object::int_t>()) {
            value = *x;
            return true;
        }
        return false;
    }
    template <typename T>
    bool value_obj_float(T& value) {
        if (auto x = node.get_if<Object::float_t>()) {
            value = *x;
            return true;
        }
        return false;
    }

    ConstObject node;
    std::stack<ConstObject> nodes;
    bool list_start;
};

} // namespace datapack
