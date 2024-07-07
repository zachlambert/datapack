#pragma once

#include "datapack/reader.hpp"
#include "datapack/writer.hpp"
#include <cstring>
#include <vector>


namespace datapack {

class BinaryWriter : public Writer {
public:
    BinaryWriter(std::vector<std::uint8_t>& data, bool use_binary_arrays=true):
        Writer(use_binary_arrays),
        data(data),
        is_array_(false),
        binary_size(0)
    {
        data.clear();
    }

    void value_i32(std::int32_t value) override { value_number(value); }
    void value_i64(std::int64_t value) override { value_number(value); }
    void value_u32(std::uint32_t value) override { value_number(value); }
    void value_u64(std::uint64_t value) override { value_number(value); }

    void value_f32(float value) override { value_number(value); }
    void value_f64(double value) override { value_number(value); }

    void value_string(const char* value) override;
    void value_bool(bool value) override;

    void enumerate(int value, const std::span<const char*>& labels) override;
    void optional_begin(bool has_value) override;
    void optional_end() override;
    void variant_begin(const char* label, const std::span<const char*>& labels) override;
    void variant_end() override {}

    void binary_data(const std::uint8_t* data, std::size_t size) override;

    void object_begin() override;
    void object_end() override;
    void object_next(const char* key) override {}

    void tuple_begin() override { object_begin(); }
    void tuple_end() override { object_end(); }
    void tuple_next() override {}

    void list_begin(bool is_array) override;
    void list_end() override;
    void list_next() override;

private:
    template <typename T>
    void value_number(T value) {
        if (is_array_) {
            if (!binary_blocks.empty()) {
                auto& top = binary_blocks.back();
                top.padding = std::max(top.padding, sizeof(T));
                while ((data.size() - top.start) % sizeof(T) != 0) {
                    data.push_back(0x00);
                }
            } else {
                binary_size += sizeof(T);
            }
        }

        std::size_t pos = data.size();
        data.resize(data.size() + sizeof(T));
        *((T*)&data[pos]) = value;
    }

    std::vector<std::uint8_t>& data;

    struct BinaryBlock {
        const std::size_t start;
        std::size_t padding;
        BinaryBlock(std::size_t start):
            start(start),
            padding(0)
        {}
    };
    bool is_array_;
    std::size_t binary_size;
    std::vector<BinaryBlock> binary_blocks;
};


template <writeable T>
std::vector<std::uint8_t> write_binary(const T& value) {
    std::vector<std::uint8_t> data;
    BinaryWriter(data).value(value);
    return data;
}

} // namespace datapack
