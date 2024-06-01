#include "datapack/util/object.hpp"
#include "datapack/encode/base64.hpp"


namespace datapack {

ObjectWriter::ObjectWriter(Object& object):
    object(object)
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

void ObjectWriter::optional(bool has_value) {
    if (!has_value) {
        set_value(std::nullopt);
    }
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


void ObjectWriter::binary(std::size_t size, const std::uint8_t* data) {
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


void ObjectWriter::list_begin() {
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



ObjectReader::ObjectReader(ConstObject object):
    node(object),
    list_start(false)
{}


void ObjectReader::value_i32(std::int32_t& value) {
    if (!value_obj_int(value)) {
        error("Incorrect value type (i32)");
    }
}

void ObjectReader::value_i64(std::int64_t& value) {
    if (!value_obj_int(value)) {
        error("Incorrect value type (i64)");
    }
}

void ObjectReader::value_u32(std::uint32_t& value) {
    if (!value_obj_int(value)) {
        error("Incorrect value type (u32)");
    }
}

void ObjectReader::value_u64(std::uint64_t& value) {
    if (!value_obj_int(value)) {
        error("Incorrect value type (u64)");
    }
}


void ObjectReader::value_f32(float& value) {
    if (value_obj_float(value)) return;
    if (value_obj_int(value)) return;
    error("Incorrect value type (f32)");
}

void ObjectReader::value_f64(double& value) {
    if (value_obj_float(value)) return;
    if (value_obj_int(value)) return;
    error("Incorrect value type (f64)");
}


void ObjectReader::value_string(std::string& value) {
    if (auto x = node.get_if<Object::str_t>()){
        value = *x;
        return;
    }
    error("Incorrect value type (string)");
}

void ObjectReader::value_bool(bool& value) {
    if (auto x = node.get_if<Object::bool_t>()){
        value = *x;
        return;
    }
    error("Incorrect value type (bool)");
}


int ObjectReader::enumerate(const std::vector<const char*>& labels) {
    auto x = node.get_if<Object::str_t>();
    if (!x) {
        error("Incorrect value type (enumerate)");
        return 0;
    }
    for (int i = 0; i < labels.size(); i++) {
        if ((*x) == labels[i]) {
            return i;
        }
    }
    error("Unknown enum label");
    return 0;
}

bool ObjectReader::optional() {
    if (node.get_if<Object::null_t>()) {
        return false;
    }
    return true;
}

const char* ObjectReader::variant_begin(const std::vector<const char*>& labels) {
    object_begin();
    object_next("type");
    std::string label;
    value_string(label);

    const char* result = nullptr;
    for (int i = 0; i < labels.size(); i++) {
        if (label == labels[i]) {
            result = labels[i];
            break;
        }
    }
    if (!result) {
        error("Unknown variant label");
    }

    object_next("value");
    return result;
}

void ObjectReader::variant_end() {
    object_end();
}


std::size_t ObjectReader::binary_size() {
    if (auto x = node.get_if<Object::binary_t>()) {
        return x->size();
    }
    if (auto x = node.get_if<Object::str_t>()) {
        return base64_decoded_length(*x);
    }
    error("Incorrect value type (binary)");
    return 0;
}

void ObjectReader::binary_data(std::uint8_t* data) {
    if (auto x = node.get_if<Object::binary_t>()) {
        std::memcpy(data, x->data(), x->size());
        return;
    }
    if (auto x = node.get_if<Object::str_t>()) {
        std::vector<std::uint8_t> decoded = base64_decode(*x);
        std::memcpy(data, decoded.data(), decoded.size());
        return;
    }
    error("Incorrect value type (binary)");
}


void ObjectReader::object_begin() {
    if (!node.get_if<Object::map_t>()) {
        error("Incorrect value type");
    }
    nodes.push(node);
    node = node.child();
}

void ObjectReader::object_end() {
    node = nodes.top();
    nodes.pop();
}

void ObjectReader::object_next(const char* key) {
    auto parent = node.parent();
    if (!parent) {
        error("Not in a map");
        return;
    }
    if (!parent.get_if<Object::map_t>()) {
        error("Not in a map");
        return;
    }
    auto next = parent[std::string(key)];
    if (!next) {
        error("Key not found");
        return;
    }
    node = next;
}


void ObjectReader::tuple_begin() {
    if (!node.get_if<Object::list_t>()) {
        error("Incorrect value type");
    }
    list_start = true;
    nodes.push(node);
    node = node.child();
}

void ObjectReader::tuple_end() {
    list_start = false;
    node = nodes.top();
    nodes.pop();
}

void ObjectReader::tuple_next() {
    if (!list_start) {
        node = node.next();
    }
    list_start = false;
    if (!node) {
        error("Tuple element missing");
    }
}


void ObjectReader::map_begin() {
    if (!node.get_if<Object::map_t>()) {
        error("Incorrect value type");
    }
    list_start = true;
    nodes.push(node);
    node = node.child();
}

void ObjectReader::map_end() {
    list_start = false;
    node = nodes.top();
    nodes.pop();
}

bool ObjectReader::map_next(std::string& key) {
    if (!list_start) {
        node = node.next();
    }
    list_start = false;
    if (!node) {
        return false;
    }

    if (node.key().empty()) {
        error("Empty map key");
    }
    key = node.key();
    return true;
}


void ObjectReader::list_begin() {
    if (!node.get_if<Object::list_t>()) {
        error("Incorrect value type");
    }
    list_start = true;
    nodes.push(node);
    node = node.child();
}

void ObjectReader::list_end() {
    list_start = false;
    node = nodes.top();
    nodes.pop();
}

bool ObjectReader::list_next() {
    if (!list_start) {
        node = node.next();
    }
    list_start = false;
    return bool(node);
}

} // namespace datapack
