#pragma once

#include "datapack/reader.hpp"
#include <vector>


namespace datapack {

class BinaryReader : public Reader {
public:
    BinaryReader(const std::span<const std::uint8_t>& data):
        Reader(true),
        data(data),
        pos(0),
        binary_depth(0),
        binary_start(0),
        trivial_list_remaining(0)
    {}

    void primitive(Primitive primitive, void* value) override;
    const char* string() override;
    int enumerate(const std::span<const char*>& labels) override;

    bool optional_begin() override;
    void optional_end() override;

    int variant_begin(const std::span<const char*>& labels) override;
    void variant_end() override {}

    std::tuple<const std::uint8_t*, std::size_t>
        binary_data(std::size_t length, std::size_t stride) override;

    void object_begin(std::size_t size) override;
    void object_end(std::size_t size) override;
    void object_next(const char* key) override {}

    void tuple_begin(std::size_t size) override {
        object_begin(size);
    }
    void tuple_end(std::size_t size) override {
        object_end(size);
    }
    void tuple_next() override {}

    void list_begin(bool is_trivial) override;
    bool list_next() override;
    void list_end() override;

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
            invalidate();
            return;
        }

        value = *((T*)&data[pos]);
        pos += sizeof(T);
    }

    void value_bool(bool& value) {
        if (pos + 1 > data.size()) {
            invalidate();
            return;
        }
        std::uint8_t value_int = *((bool*)&data[pos]);
        if (value_int >= 2) {
            invalidate();
            return;
        }
        value = value_int;
        pos++;
    }

    std::span<const std::uint8_t> data;
    std::size_t pos;
    std::size_t binary_depth;
    std::int64_t binary_start;
    int trivial_list_remaining;
};

template <readable T>
T read_binary(const std::span<const std::uint8_t>& data) {
    T result;
    BinaryReader(data).value(result);
    return result;
}

} // namespace datapack
