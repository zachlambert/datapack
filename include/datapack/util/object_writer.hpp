#pragma once
#ifndef EMBEDDED

#include "datapack/object.hpp"
#include "datapack/reader.hpp"
#include "datapack/writer.hpp"


namespace datapack {

class ObjectWriter: public Writer {
public:
    ObjectWriter(Object::Reference object);

    void primitive(Primitive primtive, const void* value) override;
    void string(const char* value) override;
    void enumerate(int value, const char* label) override;

    void optional_begin(bool has_value) override;
    void optional_end() override;
    void variant_begin(int value, const char* label) override;
    void variant_end() override;

    void binary_data(const std::uint8_t* data, std::size_t size, std::size_t stride, bool fixed_length) override;

    void object_begin(std::size_t size) override;
    void object_end(std::size_t size) override;
    void object_next(const char* key) override;

    void tuple_begin(std::size_t size) override;
    void tuple_end(std::size_t size) override;
    void tuple_next() override;

    void list_begin(bool is_trivial) override;
    void list_end() override;
    void list_next() override;

private:
    void set_value(const Object::value_t& value);

    Object::Reference object;
    std::stack<Object::Pointer> nodes;
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
#endif
