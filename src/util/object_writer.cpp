#include "datapack/util/object_writer.hpp"
#include "datapack/encode/base64.hpp"
#include <cstring>


namespace datapack {

ObjectWriter::ObjectWriter(Object& object):
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


void ObjectWriter::value_string(const std::string& value) {
    set_value(value);
}

void ObjectWriter::value_bool(bool value) {
    set_value(value);
}


void ObjectWriter::enumerate(int value, const std::vector<const char*>& labels) {
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

void ObjectWriter::variant_begin(const char* label, const std::vector<const char*>& labels) {
    object_begin();
    object_next("type");
    value_string(label);
    object_next("value");
}

void ObjectWriter::variant_end() {
    object_end();
}


void ObjectWriter::binary_data(const std::uint8_t* data, std::size_t size) {
    std::vector<std::uint8_t> vec(size);
    std::memcpy(vec.data(), data, size);
    set_value(vec);
}


void ObjectWriter::object_begin() {
    set_value(Object::map_t());
}

void ObjectWriter::object_end() {
    nodes.pop();
}

void ObjectWriter::object_next(const char* key) {
    next_key = key;
}


void ObjectWriter::tuple_begin() {
    set_value(Object::list_t());
}

void ObjectWriter::tuple_end() {
    nodes.pop();
}

void ObjectWriter::tuple_next() {
    next_key = "";
}


void ObjectWriter::map_begin() {
    set_value(Object::map_t());
}

void ObjectWriter::map_end() {
    nodes.pop();
}

void ObjectWriter::map_next(const std::string& key) {
    next_key = key;
}


void ObjectWriter::list_begin(bool is_array) {
    set_value(Object::list_t());
}

void ObjectWriter::list_end() {
    nodes.pop();
}

void ObjectWriter::list_next() {
    next_key = "";
}


void ObjectWriter::set_value(const Object::value_t& value) {
    Object next;

    if (nodes.empty()) {
        object = Object(value);
        next = object.root();
    } else {
        const auto& node = nodes.top();
        if (node.get_if<Object::map_t>()) {
            next = node.insert(next_key, value);
        } else if (node.get_if<Object::list_t>()) {
            next = node.append(value);
        } else {
            throw std::runtime_error("Shouldn't reach here");
        }
    }

    if (next.get_if<Object::map_t>() || next.get_if<Object::list_t>()) {
        nodes.push(next);
    }
}



} // namespace datapack
