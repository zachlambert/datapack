#pragma once

#include "datapack/reader.hpp"
#include "datapack/writer.hpp"
#include <cstring>
#include <vector>
#include <assert.h>
#include <micro_types/vector.hpp>


namespace datapack {


template <bool Dynamic>
class BinaryWriterGeneric : public Writer {
public:
    using data_t = std::conditional_t<Dynamic,
        std::vector<std::uint8_t>,
        mct::vector<std::uint8_t>
    >;
    BinaryWriterGeneric(data_t& data, bool trivial_as_binary=true):
        Writer(trivial_as_binary),
        data(data),
        pos(data.size()),
        binary_depth(false),
        binary_start(0),
        trivial_list_length(0)
    {}

    void primitive(Primitive primitive, const void* value) override {
        switch (primitive) {
            case Primitive::I32:
                value_number(*(const std::int32_t*)value);
                break;
            case Primitive::I64:
                value_number(*(const std::int64_t*)value);
                break;
            case Primitive::U32:
                value_number(*(const std::uint32_t*)value);
                break;
            case Primitive::U64:
                value_number(*(const std::uint64_t*)value);
                break;
            case Primitive::F32:
                value_number(*(const float*)value);
                break;
            case Primitive::F64:
                value_number(*(const double*)value);
                break;
            case Primitive::U8:
                value_number(*(const std::uint8_t*)value);
                break;
            case Primitive::BOOL:
                value_bool(*(const bool*)value);
        }
    }

    void string(const char* value) override {
        std::size_t size = std::strlen(value) + 1;
        if (!resize(pos + size)) {
            return;
        }
        strncpy((char*)&data[pos], value, size);
        pos += size;
    }

    void enumerate(int value, const char* label) override {
        value_number(value);
    }

    void optional_begin(bool has_value) override {
        value_bool(has_value);
    }
    void optional_end() override {}

    void variant_begin(int value, const char* label) override {
        value_number(value);
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

    std::span<std::uint8_t> result() const {
        return std::span(&data[0], pos);
    }

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
            data.resize(new_size);
            return true;
        }
        if constexpr(!Dynamic) {
            return data.resize(new_size);
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

    void value_bool(bool value) {
        if (!resize(pos + 1)) {
            return;
        }
        data[pos] = (value ? 0x01 : 0x00);
        pos++;
    }

    data_t& data;
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
void write_binary(const T& value, mct::vector<std::uint8_t>& data) {
    BinaryWriterStatic(data).value(value);
}

} // namespace datapack
