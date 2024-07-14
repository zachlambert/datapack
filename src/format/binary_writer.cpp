#include "datapack/format/binary_writer.hpp"


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

void BinaryWriter::binary_data(const std::uint8_t* input_data, std::size_t size) {
    value_number(std::uint64_t(size));
    if (!resize(pos + size)) {
        return;
    }
    std::memcpy(&data[pos], input_data, size);
    pos += size;
}

void BinaryWriter::trivial_begin(std::size_t size) {
    printf("trivial_begin: %zu\n", pos);
    if (binary_depth == 0) {
        printf("start\n");
        value_number<std::uint64_t>(0); // Placeholder
        binary_start = pos;
    }
    printf("at: %zu\n", pos);
    if (!pad(size)) {
        return;
    }
    binary_depth++;
    printf("data start at: %zu\n", pos);
}

void BinaryWriter::trivial_end(std::size_t size) {
    printf("trivial_end: %zu\n", pos);
    if (!pad(size)) {
        return;
    }
    binary_depth--;
    if (binary_depth == 0) {
        std::size_t binary_size = pos - binary_start;
        *((std::uint64_t*)&data[binary_start - sizeof(std::uint64_t)]) = binary_size;
    }
}

void BinaryWriter::object_begin() {
    // Do nothing
}

void BinaryWriter::object_end() {
    // Do nothing
}

void BinaryWriter::list_begin() {
    // Do nothing
}

void BinaryWriter::list_end() {
    if (binary_depth == 0) {
        value_bool(false);
    }
}

void BinaryWriter::list_next() {
    if (binary_depth == 0) {
        value_bool(true);
    }
}


} // namespace datapack
