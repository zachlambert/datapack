#include "datapack/util/object_reader.hpp"
#include "datapack/encode/base64.hpp"


namespace datapack {

ObjectReader::ObjectReader(ConstObject object):
    node(object),
    list_start(false),
    next_variant_label(nullptr)
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

bool ObjectReader::optional_begin() {
    if (node.get_if<Object::null_t>()) {
        return false;
    }
    return true;
}

void ObjectReader::optional_end() {
    // Do nothing
}

void ObjectReader::variant_begin(const std::vector<const char*>& labels) {
    object_begin();
    object_next("type");
}

bool ObjectReader::variant_match(const char* label) {
    if (auto x = node.get_if<Object::str_t>()){
        if (*x == label) {
            object_next("value");
            return true;
        }
        return false;
    }
    error("Incorrect data type");
    return false;
}

void ObjectReader::variant_end() {
    object_end();
}

std::tuple<const std::uint8_t*, std::size_t> ObjectReader::binary_data() {
    if (auto x = node.get_if<Object::binary_t>()) {
        return std::make_tuple(x->data(), x->size());
    }
    if (auto x = node.get_if<Object::str_t>()) {
        data_temp = base64_decode(*x);
        return std::make_tuple(data_temp.data(), data_temp.size());
    }
    error("Incorrect value type (binary)");
    return std::make_tuple(nullptr, 0);
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


void ObjectReader::list_begin(bool is_array) {
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
