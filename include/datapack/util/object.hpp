#pragma once

#include <stack>
#include <cstring>
#include "datapack/object.hpp"
#include "datapack/reader.hpp"
#include "datapack/writer.hpp"


namespace datapack {

class ObjectWriter: public Writer {
public:
    ObjectWriter(Object& object):
        object(object)
    {}

    void value_i32(std::int32_t value) override {
        set_value((object::int_t)value);
    }
    void value_i64(std::int64_t value) override {
        set_value((object::int_t)value);
    }
    void value_u32(std::uint32_t value) override {
        set_value((object::int_t)value);
    }
    void value_u64(std::uint64_t value) override {
        set_value((object::int_t)value);
    }

    void value_f32(float value) override {
        set_value((object::float_t)value);
    }
    void value_f64(double value) override {
        set_value((object::float_t)value);
    }

    void value_string(const std::string& value) override {
        set_value(value);
    }
    void value_bool(bool value) override {
        set_value(value);
    }

    void enumerate(int value, const std::vector<const char*>& labels) override {
        set_value(std::string(labels[value]));
    }
    void optional(bool has_value) override {
        if (!has_value) {
            set_value(std::nullopt);
        }
    }
    void variant_begin(const char* label, const std::vector<const char*>& labels) override {
        object_begin();
        object_next("type");
        value_string(label);
        object_next("value");
    }
    void variant_end() override {
        object_end();
    }

    void binary(std::size_t size, const std::uint8_t* data) override {
        std::vector<std::uint8_t> vec(size);
        std::memcpy(vec.data(), data, size);
        set_value(vec);
    }

    void object_begin() override {
        set_value(object::map_t());
    }
    void object_end() override {
        nodes.pop();
    }
    void object_next(const char* key) override {
        next_key = key;
    }

    void tuple_begin() override {
        set_value(object::list_t());
    }
    void tuple_end() override {
        nodes.pop();
    }
    void tuple_next() override {
        next_key = "";
    }

    void map_begin() override {
        set_value(object::map_t());
    }
    void map_end() override {
        nodes.pop();
    }
    void map_next(const std::string& key) override {
        next_key = key;
    }

    void list_begin() override {
        set_value(object::list_t());
    }
    void list_end() override {
        nodes.pop();
    }
    void list_next() override {
        next_key = "";
    }

private:
    void set_value(const object::value_t& value) {
        Object::Pointer next = object.null();

        if (nodes.empty()) {
            object.set_root(value);
            next = object.root();
        } else {
            const auto& node = nodes.top();
            if (node.get_if<object::map_t>()) {
                next = node.insert(next_key, value);
            } else if (node.get_if<object::list_t>()) {
                next = node.append(value);
            } else {
                throw std::runtime_error("Shouldn't reach here");
            }
        }

        if (next.get_if<object::map_t>() || next.get_if<object::list_t>()) {
            nodes.push(next);
        }
    }

    Object& object;
    std::stack<Object::Pointer> nodes;
    std::string next_key;
};


class ObjectReader: public Reader {
public:
    ObjectReader(const Object& object):
        object(object),
        node(object.root()),
        list_start(false)
    {}

    void value_i32(std::int32_t& value) override {
        if (!value_obj_int(value)) {
            error("Incorrect value type (i32)");
        }
    }
    void value_i64(std::int64_t& value) override {
        if (!value_obj_int(value)) {
            error("Incorrect value type (i64)");
        }
    }
    void value_u32(std::uint32_t& value) override {
        if (!value_obj_int(value)) {
            error("Incorrect value type (u32)");
        }
    }
    void value_u64(std::uint64_t& value) override {
        if (!value_obj_int(value)) {
            error("Incorrect value type (u64)");
        }
    }

    void value_f32(float& value) override {
        if (value_obj_float(value)) return;
        if (value_obj_int(value)) return;
        error("Incorrect value type (f32)");
    }
    void value_f64(double& value) override {
        if (value_obj_float(value)) return;
        if (value_obj_int(value)) return;
        error("Incorrect value type (f64)");
    }

    void value_string(std::string& value) override {
        if (auto x = node.get_if<object::str_t>()){
            value = *x;
            return;
        }
        error("Incorrect value type (string)");
    }
    void value_bool(bool& value) override {
        if (auto x = node.get_if<object::bool_t>()){
            value = *x;
            return;
        }
        error("Incorrect value type (bool)");
    }

    int enumerate(const std::vector<const char*>& labels) override {
        auto x = node.get_if<object::str_t>();
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
    bool optional() override {
        if (node.get_if<object::null_t>()) {
            return false;
        }
        return true;
    }
    const char* variant_begin(const std::vector<const char*>& labels) override {
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
    void variant_end() override {
        object_end();
    }

    std::size_t binary_size(std::size_t expected_size=0) override {
        if (auto x = node.get_if<object::binary_t>()) {
            return x->size();
        }
        if (auto x = node.get_if<object::str_t>()) {
            error("TODO: Handle base-64 encoded binary data");
        }
        error("Incorrect value type (binary)");
        return 0;
    }
    void binary_data(std::uint8_t* data) override {
        if (auto x = node.get_if<object::binary_t>()) {
            std::memcpy(data, x->data(), x->size());
            return;
        }
        error("Incorrect value type (binary)");
    }

    void object_begin() override {
        if (!node.get_if<object::map_t>()) {
            error("Incorrect value type");
        }
        nodes.push(node);
        node = node.child();
    }
    void object_end() override {
        node = nodes.top();
        nodes.pop();
    }
    void object_next(const char* key) override {
        auto parent = node.parent();
        if (!parent) {
            error("Not in a map");
            return;
        }
        if (!parent.get_if<object::map_t>()) {
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

    void tuple_begin() override {
        if (!node.get_if<object::list_t>()) {
            error("Incorrect value type");
        }
        list_start = true;
        nodes.push(node);
        node = node.child();
    }
    void tuple_end() override {
        list_start = false;
        node = nodes.top();
        nodes.pop();
    }
    void tuple_next() override {
        if (!list_start) {
            node = node.next();
        }
        list_start = false;
        if (!node) {
            error("Tuple element missing");
        }
    }

    void map_begin() override {
        if (!node.get_if<object::map_t>()) {
            error("Incorrect value type");
        }
        list_start = true;
        nodes.push(node);
        node = node.child();
    }
    void map_end() override {
        list_start = false;
        node = nodes.top();
        nodes.pop();
    }
    bool map_next(std::string& key) override {
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

    void list_begin() override {
        if (!node.get_if<object::list_t>()) {
            error("Incorrect value type");
        }
        list_start = true;
        nodes.push(node);
        node = node.child();
    }
    void list_end() override {
        list_start = false;
        node = nodes.top();
        nodes.pop();
    }
    bool list_next() override {
        if (!list_start) {
            node = node.next();
        }
        list_start = false;
        return bool(node);
    }

private:
    template <typename T>
    bool value_obj_int(T& value) {
        if (auto x = node.get_if<object::int_t>()) {
            value = *x;
            return true;
        }
        return false;
    }
    template <typename T>
    bool value_obj_float(T& value) {
        if (auto x = node.get_if<object::float_t>()) {
            value = *x;
            return true;
        }
        return false;
    }

    const Object& object;
    std::stack<Object::ConstPointer> nodes;
    Object::ConstPointer node;
    bool list_start;
};

} // namespace datapack
