#include "datapack/util/object_writer.hpp"
#include "datapack/encode/base64.hpp"
#include <cstring>


namespace datapack {

ObjectWriter::ObjectWriter(Object::Reference object):
    object(object),
    next_stride(0)
{}


void ObjectWriter::primitive(Primitive primitive, const void* value) {
    switch (primitive) {
        case Primitive::I32:
            set_value((Object::int_t)(*(std::int32_t*)value));
            break;
        case Primitive::I64:
            set_value((Object::int_t)(*(std::int64_t*)value));
            break;
        case Primitive::U32:
            set_value((Object::int_t)(*(std::uint32_t*)value));
            break;
        case Primitive::U64:
            set_value((Object::int_t)(*(std::uint64_t*)value));
            break;
        case Primitive::F32:
            set_value((Object::float_t)(*(float*)value));
            break;
        case Primitive::F64:
            set_value((Object::float_t)(*(double*)value));
            break;
        case Primitive::U8:
            set_value((Object::int_t)(*(uint8_t*)value));
            break;
        case Primitive::BOOL:
            set_value(*(bool*)value);
            break;
    }
}

void ObjectWriter::string(const char* value) {
    set_value(std::string(value));
}

void ObjectWriter::enumerate(int value, const char* label) {
    set_value(std::string(label));
}

void ObjectWriter::optional_begin(bool has_value) {
    if (!has_value) {
        set_value(std::nullopt);
    }
}

void ObjectWriter::optional_end() {
    // Do nothing
}

void ObjectWriter::variant_begin(int value, const char* label) {
    object_begin(0);
    object_next("type");
    string(label);
    std::string value_key = "value_" + std::string(label);
    object_next(value_key.c_str());
}

void ObjectWriter::variant_end() {
    object_end(0);
}


void ObjectWriter::binary_data(const std::uint8_t* data, std::size_t length, std::size_t stride, bool fixed_length) {
    std::vector<std::uint8_t> vec(length * stride);
    std::memcpy(vec.data(), data, length * stride);
    set_value(vec);
}


void ObjectWriter::object_begin(std::size_t size) {
    set_value(Object::map_t());
}

void ObjectWriter::object_end(std::size_t size) {
    nodes.pop();
}

void ObjectWriter::object_next(const char* key) {
    next_key = key;
}


void ObjectWriter::tuple_begin(std::size_t size) {
    set_value(Object::list_t());
}

void ObjectWriter::tuple_end(std::size_t size) {
    nodes.pop();
}

void ObjectWriter::tuple_next() {
    next_key = "";
}


void ObjectWriter::list_begin(bool is_trivial) {
    set_value(Object::list_t());
}

void ObjectWriter::list_end() {
    nodes.pop();
}

void ObjectWriter::list_next() {
    next_key = "";
}


void ObjectWriter::set_value(const Object::value_t& value) {
    Object::Pointer next;

    if (nodes.empty()) {
        object = value;
        next = object.ptr();
    } else {
        const auto& node = nodes.top();
        if (node->is_map()) {
            next = node->insert(next_key, value).ptr();
        } else if (node->is_list()) {
            next = node->append(value).ptr();
        } else {
            throw std::runtime_error("Shouldn't reach here");
        }
    }

    if (next->is_map() || next->is_list()) {
        nodes.push(next);
    }
}



} // namespace datapack
