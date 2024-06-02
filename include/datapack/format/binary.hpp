#pragma once

#include "datapack/reader.hpp"
#include "datapack/writer.hpp"
#include <vector>
#include <cstring>
#include <iostream>


namespace datapack {

class BinaryWriter : public Writer {
public:
    BinaryWriter(std::vector<std::uint8_t>& data, bool use_binary=true):
        Writer(use_binary),
        pos(0),
        data(data),
        enforce_padding(false),
        container_start(0),
        container_size(0),
        know_object_details(false),
        object_size(0),
        object_alignment(0)
    {
        data.clear();
    }

    void value_i32(std::int32_t value) override { value_number(value); }
    void value_i64(std::int64_t value) override { value_number(value); }
    void value_u32(std::uint32_t value) override { value_number(value); }
    void value_u64(std::uint64_t value) override { value_number(value); }

    void value_f32(float value) override { value_number(value); }
    void value_f64(double value) override { value_number(value); }

    void value_string(const std::string&) override;
    void value_bool(bool value) override;

    void enumerate(int value, const std::vector<const char*>& labels) override;
    void optional(bool has_value) override;
    void variant_begin(const char* label, const std::vector<const char*>& labels) override;
    void variant_end() override {}

    void binary(std::size_t size, const std::uint8_t* data, std::size_t stride) override;

    void object_begin() override {
        if (enforce_padding && !know_object_details) {
            object_size = 0;
            object_alignment = 0;
        }
    }
    void object_end() override {
        if (enforce_padding) {
            while (data.size() % object_alignment != 0) {
                data.push_back(0x00);
            }
        }
    }
    void object_next(const char* key) override {}

    void tuple_begin() override {}
    void tuple_end() override {}
    void tuple_next() override {}

    void map_begin() override;
    void map_end() override;
    void map_next(const std::string& key) override;

    void list_begin() override;
    void list_end() override;
    void list_next() override;

private:
    template <typename T>
    void value_number(T value) {
        if (enforce_padding) {
            while (data.size() % sizeof(T) != 0) {
                data.push_back(0x00);
            }
            if (!know_object_details) {
                object_size += sizeof(T);
                object_alignment = std::max(object_alignment, sizeof(T));
            }
        }
        std::size_t pos = data.size();
        data.resize(data.size() + sizeof(T));
        *((T*)&data[pos]) = value;
    }

    std::size_t pos;
    std::vector<std::uint8_t>& data;
    bool enforce_padding;
    std::size_t container_start;
    std::size_t container_size;
    bool know_object_details;
    std::size_t object_size;
    std::size_t object_alignment;
};


class BinaryReader : public Reader {
public:
    BinaryReader(const std::vector<std::uint8_t>& data, bool use_binary=true):
        Reader(use_binary),
        data(data),
        pos(0),
        next_binary_size(0),
        next_variant_label(nullptr)
    {}

    void value_i32(std::int32_t& value) override { value_number(value); }
    void value_i64(std::int64_t& value) override { value_number(value); }
    void value_u32(std::uint32_t& value) override { value_number(value); }
    void value_u64(std::uint64_t& value) override { value_number(value); }

    void value_f32(float& value) override { value_number(value); }
    void value_f64(double& value) override { value_number(value); }

    void value_string(std::string& value) override;
    void value_bool(bool& value) override;

    int enumerate(const std::vector<const char*>& labels) override;
    bool optional() override;
    void variant_begin(const std::vector<const char*>& labels) override;
    bool variant_match(const char* label) override;
    void variant_end() override {}

    std::size_t binary_size(std::size_t stride) override;
    void binary_data(std::uint8_t* data) override;

    void object_begin() override {}
    void object_end() override {}
    void object_next(const char* key) override {}

    void tuple_begin() override {}
    void tuple_end() override {}
    void tuple_next() override {}

    void map_begin() override;
    void map_end() override;
    bool map_next(std::string& key) override;

    void list_begin() override;
    void list_end() override;
    bool list_next() override;

private:
    template <typename T>
    void value_number(T& value) {
        if (pos + sizeof(T) > data.size()) {
            error("Input data is too short");
            return;
        }
        value = *((T*)&data[pos]);
        pos += sizeof(T);
    }

    const std::vector<std::uint8_t>& data;
    std::size_t pos;
    std::size_t next_binary_size;
    const char* next_variant_label;
};

template <readable T>
T read_binary(const std::vector<std::uint8_t>& data) {
    T result;
    BinaryReader(data).value(result);
    return result;
}

template <writeable T>
std::vector<std::uint8_t> write_binary(const T& value) {
    std::vector<std::uint8_t> data;
    BinaryWriter(data).value(value);
    return data;
}

} // namespace datapack
