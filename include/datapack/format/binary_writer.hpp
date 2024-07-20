#pragma once

#include "datapack/reader.hpp"
#include "datapack/writer.hpp"
#include <cstring>
#include <vector>
#include <assert.h>


namespace datapack {


template <bool Dynamic>
class BinaryWriterGeneric : public Writer {
public:
    BinaryWriterGeneric(std::vector<std::uint8_t>& data, bool trivial_as_binary=true)
        requires Dynamic:
        Writer(trivial_as_binary),
        data(data),
        pos(data.size()),
        binary_depth(false),
        binary_start(0),
        trivial_list_length(0)
    {}

    BinaryWriterGeneric(const std::span<std::uint8_t>& data, bool trivial_as_binary=true)
        requires (!Dynamic):
        Writer(trivial_as_binary),
        data(data),
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

    void value_string(const char* value) override {
        std::size_t size = std::strlen(value) + 1;
        if (!resize(pos + size)) {
            return;
        }
        strncpy((char*)&data[pos], value, size);
        pos += size;
    }
    void value_bool(bool value) override {
        if (!resize(pos + 1)) {
            return;
        }
        data[pos] = (value ? 0x01 : 0x00);
        pos++;
    }

    void enumerate(int value, const std::span<const char*>& labels) override {
        value_number(value);
    }

    void optional_begin(bool has_value) override {
        value_bool(has_value);
    }
    void optional_end() override {}

    void variant_begin(const char* label, const std::span<const char*>& labels) override {
        value_string(label);
    }
    void variant_end() override {}

    void binary_data(
        const std::uint8_t* input_data,
        std::size_t length,
        std::size_t stride,
        bool fixed_length) override
    {
        std::size_t size = length * stride;
        if (!fixed_length) {
            value_number(std::uint64_t(length));
        }
        if (!resize(pos + size)) {
            return;
        }
        std::memcpy(&data[pos], input_data, size);
        pos += size;
    }

    void object_begin(std::size_t size) override {
        if (size == 0){
            return;
        }
        if (binary_depth == 0) {
            binary_start = pos;
        }
        pad(size);
        binary_depth++;
    }

    void object_end(std::size_t size) override {
        if (size == 0) {
            return;
        }
        pad(size);
        binary_depth--;
    }

    void object_next(const char* key) override {}

    void list_begin(bool is_trivial) override {
        if (binary_depth != 0) {
            assert(false);
            // TODO: Handle er
            // set_error("Cannot start a list inside a trivial list");
            return;
        }
        if (!is_trivial) {
            return;
        }

        trivial_list_length = 0;
        value_number(std::uint64_t(0)); // Placeholder

        binary_depth++;
        binary_start = pos;
    }

    void list_end() override {
        if (binary_depth == 0) {
            value_bool(false);
            return;
        }

        *(std::uint64_t*)&data[binary_start - sizeof(std::uint64_t)] = trivial_list_length;
        binary_depth--;
        assert(binary_depth == 0);
    }

    void list_next() override {
        if (binary_depth == 0) {
            value_bool(true);
            return;
        }
        trivial_list_length++;
    }

    void tuple_begin(std::size_t size) override { object_begin(size); }
    void tuple_end(std::size_t size) override { object_end(size); }
    void tuple_next() override {}

private:
    bool pad(std::size_t size) {
        if ((pos-binary_start) % size != 0) {
            pos += (size - (pos-binary_start) % size);
            return resize(pos);
        }
        return true;
    }

    bool resize(std::size_t new_size) {
        if constexpr(Dynamic) {
            if (data.size() < new_size) {
                data.resize(new_size);
            }
            return true;
        }
        if constexpr(!Dynamic) {
            return new_size <= data.size();
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

    std::conditional_t<
        Dynamic,
        std::vector<std::uint8_t>&,
        std::span<std::uint8_t>
    > data;
    std::size_t pos;
    std::size_t binary_depth;
    std::size_t binary_start;
    std::size_t trivial_list_length;
};

using BinaryWriter = BinaryWriterGeneric<true>;
using BinaryWriterStatic = BinaryWriterGeneric<false>;


template <writeable T>
std::vector<std::uint8_t> write_binary(const T& value) {
    std::vector<std::uint8_t> data;
    BinaryWriter(data).value(value);
    return data;
}

template <writeable T>
void write_binary(const T& value, std::span<std::uint8_t> data) {
    BinaryWriterStatic(data).value(value);
}

} // namespace datapack
