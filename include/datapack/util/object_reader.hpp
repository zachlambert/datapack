#pragma once
#ifndef EMBEDDED

#include "datapack/object.hpp"
#include "datapack/reader.hpp"


namespace datapack {

class ObjectReader: public Reader {
public:
    ObjectReader(Object::ConstReference object);

    void integer(IntType type, void* value) override;
    void floating(FloatType type, void* value) override;
    bool boolean() override;
    const char* string() override;
    int enumerate(const std::span<const char*>& labels) override;
    std::tuple<const std::uint8_t*, std::size_t> binary(std::size_t length, std::size_t stride) override;

    bool optional_begin() override;
    void optional_end() override;

    int variant_begin(const std::span<const char*>& labels) override;
    void variant_end() override;

    void object_begin(std::size_t size) override;
    void object_end(std::size_t size) override;
    void object_next(const char* key) override;

    void tuple_begin(std::size_t size) override;
    void tuple_end(std::size_t size) override;
    void tuple_next() override;

    void list_begin(bool is_trivial) override;
    bool list_next() override;
    void list_end() override;

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
