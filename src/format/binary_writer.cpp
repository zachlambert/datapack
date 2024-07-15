#include "datapack/format/binary_writer.hpp"
#include <assert.h>


namespace datapack {

void BinaryWriter::value_string(const char* value) {
    std::size_t size = std::strlen(value) + 1;
    if (!resize(pos + size)) {
        return;
    }
    strncpy((char*)&data[pos], value, size);
    pos += size;
}

void BinaryWriter::value_bool(bool value) {
    if (!resize(pos + 1)) {
        return;
    }
    data[pos] = (value ? 0x01 : 0x00);
    pos++;
}


void BinaryWriter::enumerate(int value, const std::span<const char*>& labels) {
    value_number(value);
}

void BinaryWriter::optional_begin(bool has_value) {
    value_bool(has_value);
}

void BinaryWriter::optional_end() {
    // Nothing required
}

void BinaryWriter::variant_begin(const char* label, const std::span<const char*>& labels) {
    value_string(label);
}

void BinaryWriter::binary_data(const std::uint8_t* input_data, std::size_t length, std::size_t stride) {
    std::size_t size = length * stride;
    value_number(std::uint64_t(length));
    if (!resize(pos + size)) {
        return;
    }
    std::memcpy(&data[pos], input_data, size);
    pos += size;
}

void BinaryWriter::object_begin(std::size_t size) {
    if (size == 0){
        return;
    }
    if (binary_depth == 0) {
        binary_start = pos;
    }
    pad(size);
    binary_depth++;
}

void BinaryWriter::object_end(std::size_t size) {
    if (size == 0) {
        return;
    }
    pad(size);
    binary_depth--;
}

void BinaryWriter::list_begin(bool is_trivial) {
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

void BinaryWriter::list_end() {
    if (binary_depth == 0) {
        value_bool(false);
        return;
    }

    *(std::uint64_t*)&data[binary_start - sizeof(std::uint64_t)] = trivial_list_length;
    binary_depth--;
    assert(binary_depth == 0);
}

void BinaryWriter::list_next() {
    if (binary_depth == 0) {
        value_bool(true);
        return;
    }
    trivial_list_length++;
}


} // namespace datapack
