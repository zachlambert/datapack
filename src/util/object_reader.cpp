#include "datapack/util/object_reader.hpp"
#include "datapack/encode/base64.hpp"


namespace datapack {

ObjectReader::ObjectReader(Object::ConstReference object):
    node(object.ptr()),
    list_start(false),
    next_variant_label(nullptr)
{}


void ObjectReader::primitive(Primitive primitive, void* value) {
    bool valid = true;
    switch (primitive) {
        case Primitive::I32:
            valid = value_obj_int(*(std::uint32_t*)value);
            break;
        case Primitive::I64:
            valid = value_obj_int(*(std::uint32_t*)value);
            break;
        case Primitive::U32:
            valid = value_obj_int(*(std::uint32_t*)value);
            break;
        case Primitive::U64:
            valid = value_obj_int(*(std::uint32_t*)value);
            break;
        case Primitive::F32:
            valid =
                value_obj_float(*(float*)value)
                || value_obj_int(*(float*)value);
            break;
        case Primitive::F64:
            valid =
                value_obj_float(*(float*)value)
                || value_obj_int(*(float*)value);
            break;
        case Primitive::U8:
            valid = value_obj_int(*(std::uint8_t*)value);
            break;
        case Primitive::BOOL:
            if (auto x = node->get_bool()) {
                *(bool*)value = *x;
            }
            invalidate();
            return;
    }
    if (!valid) {
        invalidate();
    }
}

const char* ObjectReader::string() {
    if (auto x = node->get_string()) {
        return x->c_str();
    }
    invalidate();
    return nullptr;
}

int ObjectReader::enumerate(const std::span<const char*>& labels) {
    auto x = node->get_string();
    if (!x) {
        invalidate();
        return 0;
    }
    for (int i = 0; i < labels.size(); i++) {
        if ((*x) == labels[i]) {
            return i;
        }
    }
    invalidate();
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

int ObjectReader::variant_begin(const std::span<const char*>& labels) {
    object_begin(0);
    object_next("type");
    if (auto x = node->get_string()) {
        for (int i = 0; i < labels.size(); i++) {
            if (labels[i] == *x) {
                return i;
            }
        }
    }
    invalidate();
    return 0;
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
            invalidate();
            return { nullptr, 0 };
        }
        if (length != 0 && length * stride != x->size()) {
            invalidate();
            return { nullptr, 0 };
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
        invalidate();
        return { nullptr, 0 };
    }

    if (size % stride != 0) {
        invalidate();
        return { nullptr, 0 };
    } else if (length == 0) {
        length = size / stride;
    } else if (length * stride != size) {
        invalidate();
        return { nullptr, 0 };
    }

    return std::make_tuple(data, length);
}

void ObjectReader::object_begin(std::size_t size) {
    if (!node->is_map()) {
        invalidate();
        return;
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
        invalidate();
        return;
    }
    if (!parent->is_map()) {
        invalidate();
        return;
    }
    auto next = parent->find(std::string(key));
    if (!next) {
        invalidate();
        return;
    }
    node = next;
}


void ObjectReader::tuple_begin(std::size_t size) {
    if (!node->is_list()) {
        invalidate();
        return;
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
        invalidate();
        return;
    }
}


void ObjectReader::list_begin(bool is_trivial) {
    if (!node->is_list()) {
        invalidate();
        return;
    }
    list_start = true;
    nodes.push(node);
    node = node.child();
}

bool ObjectReader::list_next() {
    if (!list_start) {
        node = node.next();
    }
    list_start = false;
    return bool(node);
}

void ObjectReader::list_end() {
    list_start = false;
    node = nodes.top();
    nodes.pop();
}

} // namespace datapack
