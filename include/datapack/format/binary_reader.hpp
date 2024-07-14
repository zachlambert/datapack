#pragma once

#include "datapack/reader.hpp"
#include <vector>


namespace datapack {

class BinaryReader : public Reader {
public:
    BinaryReader(const std::span<const std::uint8_t>& data, bool trivial_as_binary=true):
        Reader(trivial_as_binary, true, false),
        data(data),
        pos(0),
        binary_depth(0),
        binary_start(0),
        binary_end(0)
    {}

    void value_i32(std::int32_t& value) override { value_number(value); }
    void value_i64(std::int64_t& value) override { value_number(value); }
    void value_u32(std::uint32_t& value) override { value_number(value); }
    void value_u64(std::uint64_t& value) override { value_number(value); }

    void value_f32(float& value) override { value_number(value); }
    void value_f64(double& value) override { value_number(value); }

    const char* value_string() override;
    void value_bool(bool& value) override;

    int enumerate(const std::span<const char*>& labels) override;
    bool optional_begin() override;
    void optional_end() override;
    void variant_begin(const std::span<const char*>& labels) override;
    bool variant_match(const char* label) override;
    void variant_end() override {}

    std::tuple<const std::uint8_t*, std::size_t> binary_data() override;
    void trivial_begin(std::size_t size) override;
    void trivial_end(std::size_t size) override;

    void object_begin() override;
    void object_end() override;
    void object_next(const char* key) override {}

    void tuple_begin() override {
        object_begin();
    }
    void tuple_end() override {
        object_end();
    }
    void tuple_next() override {}

    void list_begin() override;
    void list_end() override;
    bool list_next() override;

private:
    void pad(std::size_t size) {
        if ((pos-binary_start) % size != 0) {
            pos += (size - (pos-binary_start) % size);
        }
    }
    template <typename T>
    void value_number(T& value) {
        if (binary_depth > 0) {
            pad(sizeof(T));
        }
        if (pos + sizeof(T) > data.size()) {
            set_error("Input data is too short");
            return;
        }

        value = *((T*)&data[pos]);
        pos += sizeof(T);
    }

    std::span<const std::uint8_t> data;
    std::size_t pos;
    std::size_t binary_depth;
    std::int64_t binary_start;
    std::int64_t binary_end;
};

template <readable T>
T read_binary(const std::span<const std::uint8_t>& data) {
    T result;
    BinaryReader(data).value(result);
    return result;
}

} // namespace datapack
