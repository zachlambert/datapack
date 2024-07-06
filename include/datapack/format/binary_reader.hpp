#pragma once

#include "datapack/reader.hpp"
#include "datapack/types.hpp"


namespace datapack {

class BinaryReader : public Reader {
public:
    BinaryReader(const vector_t<std::uint8_t>& data, bool use_binary_arrays=true):
        Reader(use_binary_arrays, true, false),
        data(data),
        pos(0),
        is_array_(false),
        binary_remaining(0)
    {}

    void value_i32(std::int32_t& value) override { value_number(value); }
    void value_i64(std::int64_t& value) override { value_number(value); }
    void value_u32(std::uint32_t& value) override { value_number(value); }
    void value_u64(std::uint64_t& value) override { value_number(value); }

    void value_f32(float& value) override { value_number(value); }
    void value_f64(double& value) override { value_number(value); }

    void value_string(string_t& value) override;
    void value_bool(bool& value) override;

    int enumerate(const vector_t<const char*>& labels) override;
    bool optional_begin() override;
    void optional_end() override;
    void variant_begin(const vector_t<const char*>& labels) override;
    bool variant_match(const char* label) override;
    void variant_end() override {}

    std::tuple<const std::uint8_t*, std::size_t> binary_data() override;

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

    void map_begin() override {}
    void map_end() override {}
    bool map_next(string_t& key) override;

    void list_begin(bool is_array) override;
    void list_end() override;
    bool list_next() override;

private:
    template <typename T>
    void value_number(T& value) {
        if (is_array_) {
            if (!binary_blocks.empty()) {
                auto& top = binary_blocks.back();
                top.padding = std::max(top.padding, sizeof(T));
                while ((pos - top.start) % sizeof(T) != 0) {
                    pos++;
                }
            } else {
                binary_remaining -= sizeof(T);
            }
        }

        if (pos + sizeof(T) > data.size()) {
            error("Input data is too short");
            return;
        }
        value = *((T*)&data[pos]);
        pos += sizeof(T);
    }

    const vector_t<std::uint8_t>& data;
    std::size_t pos;

    struct BinaryBlock {
        const std::size_t start;
        std::size_t padding;
        BinaryBlock(std::size_t start):
            start(start),
            padding(0)
        {}
    };
    bool is_array_;
    std::int64_t binary_remaining;
    vector_t<BinaryBlock> binary_blocks;
};

template <readable T>
T read_binary(const vector_t<std::uint8_t>& data) {
    T result;
    BinaryReader(data).value(result);
    return result;
}

} // namespace datapack
