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
        set_value((double)value);
    }
    void value_i64(std::int64_t value) override {
        set_value((double)value);
    }
    void value_u32(std::uint32_t value) override {
        set_value((double)value);
    }
    void value_u64(std::uint64_t value) override {
        set_value((double)value);
    }

    void value_f32(float value) override {
        set_value((double)value);
    }
    void value_f64(double value) override {
        set_value((double)value);
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


#if 0
class ObjectReader: public Reader {
public:
    ObjectReader();

    void value_i32(std::int32_t& value) override;
    void value_i64(std::int64_t& value) override;
    void value_u32(std::uint32_t& value) override;
    void value_u64(std::uint64_t& value) override;

    void value_f32(float& value) override;
    void value_f64(double& value) override;

    void value_string(std::string& value) override;
    void value_bool(bool& value) override;

    int enumerate(const std::vector<const char*>& labels) override;
    bool optional() override;
    const char* variant(const std::vector<const char*>& labels) override;

    std::size_t binary_size(std::size_t expected_size=0) override;
    void binary_data(std::uint8_t* data) override;

    void object_begin() override;
    void object_end() override;
    void object_next(const char* key) override;

    void tuple_begin() override;
    void tuple_end() override;
    void tuple_next() override;

    void map_begin() override;
    void map_end() override;
    bool map_next(std::string& key) override;

    void list_begin() override;
    void list_end() override;
    bool list_next() override;

private:
    int container_counter;
    std::size_t next_binary_size;
};
#endif


} // namespace datapack
