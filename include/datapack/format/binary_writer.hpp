#pragma once

#include "datapack/reader.hpp"
#include "datapack/writer.hpp"
#include <cstring>
#include <vector>


namespace datapack {


class BinaryWriter : public Writer {
public:
    BinaryWriter(std::vector<std::uint8_t>& data, bool trivial_as_binary=true):
        Writer(trivial_as_binary),
        data_variable(&data),
        data(nullptr),
        pos(0),
        binary_depth(false),
        binary_start(0),
        trivial_list_length(0)
    {}
    BinaryWriter(std::span<std::uint8_t> data, bool trivial_as_binary=true):
        Writer(trivial_as_binary),
        data_variable(nullptr),
        data_fixed(data),
        data(data.data()),
        pos(0),
        binary_depth(false),
        binary_start(0),
        trivial_list_length(0)
    {}

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

    void binary_data(const std::uint8_t* data, std::size_t length, std::size_t stride, bool fixed_length) override;

    void object_begin(std::size_t size) override;
    void object_end(std::size_t size) override;
    void object_next(const char* key) override {}

    void tuple_begin(std::size_t size) override { object_begin(size); }
    void tuple_end(std::size_t size) override { object_end(size); }
    void tuple_next() override {}

    void list_begin(bool is_trivial) override;
    void list_end() override;
    void list_next() override;

private:
    bool pad(std::size_t size) {
        if ((pos-binary_start) % size != 0) {
            pos += (size - (pos-binary_start) % size);
            return resize(pos);
        }
        return true;
    }

    bool resize(std::size_t new_size) {
        if (data_variable) {
            data_variable->resize(new_size);
            data = data_variable->data();
            return true;
        } else {
            return new_size <= data_fixed.size();
        }
    }

    template <typename T>
    void value_number(T value) {
        if (binary_depth > 0) {
            if (!pad(sizeof(T))) {
                return;
            }
        }
        if (!resize(pos + sizeof(T))) {
            return;
        }

        *((T*)&data[pos]) = value;
        pos += sizeof(T);
    }

    std::vector<std::uint8_t>* data_variable;
    std::span<std::uint8_t> data_fixed;
    std::uint8_t* data;
    std::size_t pos;
    std::size_t binary_depth;
    std::size_t binary_start;
    std::size_t trivial_list_length;
};


template <writeable T>
std::vector<std::uint8_t> write_binary(const T& value) {
    std::vector<std::uint8_t> data;
    BinaryWriter(data).value(value);
    return data;
}

template <writeable T>
void write_binary(const T& value, std::span<std::uint8_t> data) {
    BinaryWriter(data).value(value);
}

} // namespace datapack
