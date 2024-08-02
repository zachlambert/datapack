#include "datapack/util/object_reader.hpp"
#include "datapack/encode/base64.hpp"


namespace datapack {

ObjectReader::ObjectReader(Object::ConstReference object):
    node(object.iter()),
    list_start(false),
    next_variant_label(nullptr)
{}


void ObjectReader::value_i32(std::int32_t& value) {
    if (!value_obj_int(value)) {
        set_error("Incorrect value type (i32)");
    }
}

void ObjectReader::value_i64(std::int64_t& value) {
    if (!value_obj_int(value)) {
        set_error("Incorrect value type (i64)");
    }
}

void ObjectReader::value_u32(std::uint32_t& value) {
    if (!value_obj_int(value)) {
        set_error("Incorrect value type (u32)");
    }
}

void ObjectReader::value_u64(std::uint64_t& value) {
    if (!value_obj_int(value)) {
        set_error("Incorrect value type (u64)");
    }
}


void ObjectReader::value_f32(float& value) {
    if (value_obj_float(value)) return;
    if (value_obj_int(value)) return;
    set_error("Incorrect value type (f32)");
}

void ObjectReader::value_f64(double& value) {
    if (value_obj_float(value)) return;
    if (value_obj_int(value)) return;
    set_error("Incorrect value type (f64)");
}


const char* ObjectReader::value_string() {
    if (auto x = node->get_string()) {
        return x->c_str();
    }
    set_error("Incorrect value type (string)");
    return nullptr;
}

void ObjectReader::value_bool(bool& value) {
    if (auto x = node->get_bool()) {
        value = *x;
        return;
    }
    set_error("Incorrect value type (bool)");
}


int ObjectReader::enumerate(const std::span<const char*>& labels) {
    auto x = node->get_string();
    if (!x) {
        set_error("Incorrect value type (enumerate)");
        return 0;
    }
    for (int i = 0; i < labels.size(); i++) {
        if ((*x) == labels[i]) {
            return i;
        }
    }
    set_error("Unknown enum label");
    return 0;
}

bool ObjectReader::optional_begin() {
    if (node->is_null()) {
        return false;
    }
    return true;
}

void ObjectReader::optional_end() {
    // Do nothing
}

void ObjectReader::variant_begin(const std::span<const char*>& labels) {
    object_begin(0);
    object_next("type");
}

bool ObjectReader::variant_match(const char* label) {
    if (auto x = node->get_string()) {
        if (*x == label) {
            std::string value_key = "value_" + std::string(label);
            object_next(value_key.c_str());
            return true;
        }
        return false;
    }
    set_error("Incorrect data type");
    return false;
}

void ObjectReader::variant_end() {
    object_end(0);
}

std::tuple<const std::uint8_t*, std::size_t> ObjectReader::binary_data(
    std::size_t length, std::size_t stride)
{
    const std::uint8_t* data = nullptr;
    std::size_t size = 0;

    if (auto x = node->get_binary()) {
        if (x->size() % stride != 0) {
            set_error("Invalid binary, size not a multiple of stride");
        }
        if (length != 0 && length * stride != x->size()) {
            set_error("Invalid binary, size not a multiple of stride");
        }
        data = x->data();
        size = x->size();
    }
    else if (auto x = node->get_string()) {
        data_temp = base64_decode(*x);
        data = data_temp.data();
        size = data_temp.size();
    }
    else {
        set_error("Incorrect value type (binary)");
    }

    if (size % stride != 0) {
        set_error("Invalid binary data, size not a multiple of the stride");
        length = 0;
    } else if (length == 0) {
        length = size / stride;
    } else if (length * stride != size) {
        set_error("Invalid binary data, size didn't match expected length");
        length = 0;
    }

    return std::make_tuple(data, length);
}

void ObjectReader::object_begin(std::size_t size) {
    if (!node->is_map()) {
        set_error("Incorrect value type");
    }
    nodes.push(node);
    node = node.child();
}

void ObjectReader::object_end(std::size_t size) {
    node = nodes.top();
    nodes.pop();
}

void ObjectReader::object_next(const char* key) {
    auto parent = node.parent();
    if (!parent) {
        set_error("Not in a map");
        return;
    }
    if (!parent->is_map()) {
        set_error("Not in a map");
        return;
    }
    auto next = parent->find(std::string(key));
    if (!next) {
        set_error("Key not found");
        return;
    }
    node = next;
}


void ObjectReader::tuple_begin(std::size_t size) {
    if (!node->is_list()) {
        set_error("Incorrect value type");
    }
    list_start = true;
    nodes.push(node);
    node = node.child();
}

void ObjectReader::tuple_end(std::size_t size) {
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
        set_error("Tuple element missing");
    }
}


void ObjectReader::list_begin(bool is_trivial) {
    if (!node->is_list()) {
        set_error("Incorrect value type");
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
