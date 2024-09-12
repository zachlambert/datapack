#pragma once

#include "datapack/reader.hpp"
#include "datapack/writer.hpp"
#include <cstring>
#include <vector>
#include <assert.h>
#include <micro_types/vector.hpp>


namespace datapack {


template <bool Dynamic>
class BinaryWriter_ : public Writer {
public:
    using data_t = std::conditional_t<Dynamic,
        std::vector<std::uint8_t>,
        mct::vector<std::uint8_t>
    >;
    BinaryWriter_(data_t& data, bool trivial_as_binary=true):
        Writer(trivial_as_binary),
        data(data),
        pos(data.size()),
        binary_depth(false),
        binary_start(0),
        trivial_list_length(0)
    {}

    void integer(IntType type, const void* value) override;
    void floating(FloatType type, const void* value) override;
    void boolean(bool value) override;
    void string(const char* value) override;
    void enumerate(int value, const char* label) override;
    void binary(
        const std::uint8_t* input_data,
        std::size_t length,
        std::size_t stride,
        bool fixed_length) override;

    void optional_begin(bool has_value) override;
    void optional_end() override {}

    void variant_begin(int value, const char* label) override;
    void variant_end() override {}

    void object_begin(std::size_t size) override;
    void object_next(const char* key) override {};
    void object_end(std::size_t size) override;

    void list_begin(bool is_trivial) override;
    void list_next() override;
    void list_end() override;

    void tuple_begin(std::size_t size) override { object_begin(size); }
    void tuple_next() override {}
    void tuple_end(std::size_t size) override { object_end(size); }

    std::span<std::uint8_t> result() const {
        return std::span(&data[0], pos);
    }

private:
    bool pad(std::size_t size);
    bool resize(std::size_t new_size);
    template <typename T>
    void value_number(T value);
    void value_bool(bool value);

    data_t& data;
    std::size_t pos;
    std::size_t binary_depth;
    std::size_t binary_start;
    std::size_t trivial_list_length;
};

using BinaryWriter = BinaryWriter_<true>;
using BinaryWriterStatic = BinaryWriter_<false>;


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
