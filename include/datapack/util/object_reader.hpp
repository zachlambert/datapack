#pragma once
#ifndef EMBEDDED

#include "datapack/object.hpp"
#include "datapack/reader.hpp"


namespace datapack {

class ObjectReader: public Reader {
public:
    ObjectReader(Object::ConstReference object);

    void value_i32(std::int32_t& value) override;
    void value_i64(std::int64_t& value) override;
    void value_u32(std::uint32_t& value) override;
    void value_u64(std::uint64_t& value) override;

    void value_f32(float& value) override;
    void value_f64(double& value) override;

    const char* value_string(const char* current_value) override;
    void value_bool(bool& value) override;

    int enumerate(const std::span<const char*>& labels) override;
    bool optional_begin(bool current_has_value) override;
    void optional_end() override;
    void variant_begin(const std::span<const char*>& labels) override;
    bool variant_match(const char* label) override;
    void variant_end() override;

    std::tuple<const std::uint8_t*, std::size_t> binary_data(std::size_t length, std::size_t stride) override;

    void object_begin(std::size_t size) override;
    void object_end(std::size_t size) override;
    void object_next(const char* key) override;

    void tuple_begin(std::size_t size) override;
    void tuple_end(std::size_t size) override;
    void tuple_next() override;

    void list_begin(bool is_trivial) override;
    void list_end() override;
    ListNext list_next(bool has_next) override;

private:
    template <typename T>
    bool value_obj_int(T& value) {
        if (auto x = node->get_int()) {
            value = *x;
            return true;
        }
        return false;
    }
    template <typename T>
    bool value_obj_float(T& value) {
        if (auto x = node->get_float()) {
            value = *x;
            return true;
        }
        return false;
    }

    Object::ConstPointer node;
    std::stack<Object::ConstPointer> nodes;
    bool list_start;
    const char* next_variant_label;
    std::vector<std::uint8_t> data_temp;
};

template <readable T>
T read_object(Object::ConstReference object) {
    T result;
    ObjectReader(object).value(result);
    return result;
}

} // namespace datapack
#endif
