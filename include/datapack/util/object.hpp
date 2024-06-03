#pragma once

#include <stack>
#include <cstring>
#include "datapack/object.hpp"
#include "datapack/reader.hpp"
#include "datapack/writer.hpp"
#include "datapack/schema.hpp"


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

    void binary_data(const std::uint8_t* data, std::size_t size) override;

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
    std::size_t next_stride;
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
    void variant_begin(const std::vector<const char*>& labels) override;
    bool variant_match(const char* label) override;
    void variant_end() override;

    std::tuple<const std::uint8_t*, std::size_t> binary_data() override;

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
    const char* next_variant_label;
    std::vector<std::uint8_t> data_temp;
};

template <readable T>
T read_object(ConstObject object) {
    T result;
    ObjectReader(object).value(result);
    return result;
}

template <writeable T>
Object write_object(const T& value) {
    Object object;
    ObjectWriter(object).value(value);
    return object;
}

Object read_object(const Schema& schema, const std::vector<std::uint8_t>& data);

} // namespace datapack
