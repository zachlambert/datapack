#include "datapack/util/object_writer.hpp"
#include "datapack/encode/base64.hpp"
#include <cstring>


namespace datapack {

ObjectWriter::ObjectWriter(Object::Reference object):
    object(object),
    next_stride(0)
{}


void ObjectWriter::value_i32(std::int32_t value) {
    set_value((Object::int_t)value);
}

void ObjectWriter::value_i64(std::int64_t value) {
    set_value((Object::int_t)value);
}

void ObjectWriter::value_u32(std::uint32_t value) {
    set_value((Object::int_t)value);
}

void ObjectWriter::value_u64(std::uint64_t value) {
    set_value((Object::int_t)value);
}


void ObjectWriter::value_f32(float value) {
    set_value((Object::float_t)value);
}

void ObjectWriter::value_f64(double value) {
    set_value((Object::float_t)value);
}


void ObjectWriter::value_string(const char* value) {
    set_value(std::string(value));
}

void ObjectWriter::value_bool(bool value) {
    set_value(value);
}


void ObjectWriter::enumerate(int value, const std::span<const char*>& labels) {
    set_value(std::string(labels[value]));
}

void ObjectWriter::optional_begin(bool has_value) {
    if (!has_value) {
        set_value(std::nullopt);
    }
}

void ObjectWriter::optional_end() {
    // Do nothing
}

void ObjectWriter::variant_begin(const char* label, const std::span<const char*>& labels) {
    object_begin(0);
    object_next("type");
    value_string(label);
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
