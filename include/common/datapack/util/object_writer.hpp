#pragma once

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
    void optional_begin(bool has_value) override;
    void optional_end() override;
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

    void list_begin(bool is_array) override;
    void list_end() override;
    void list_next() override;

private:
    void set_value(const Object::value_t& value);

    Object& object;
    std::stack<Object> nodes;
    std::string next_key;
    std::size_t next_stride;
};



template <writeable T>
Object write_object(const T& value) {
    Object object;
    ObjectWriter(object).value(value);
    return object;
}

} // namespace datapack
