#include "datapack/format/binary_reader.hpp"

#include <cstring>

namespace datapack {

const char* BinaryReader::value_string() {
    std::size_t max_len = data.size() - pos;
    std::size_t len = strnlen((char*)&data[pos], max_len);
    if (len == max_len) {
        set_error("Unterminated string");
    }
    const char* result = (char*)&data[pos];
    pos += (len + 1);
    return result;
}

void BinaryReader::value_bool(bool& value) {
    if (pos + 1 > data.size()) {
        set_error("Input data too short");
    }
    std::uint8_t byte = data[pos];
    pos++;
    if (byte >= 0x02) {
        set_error("Unexpected byte for bool");
    }
    value = (byte == 0x01);
}


int BinaryReader::enumerate(const std::span<const char*>& labels) {
    int value = -1;
    value_number(value);
    return value;
}

bool BinaryReader::optional_begin() {
    bool has_value;
    value_bool(has_value);
    return has_value;
}

void BinaryReader::optional_end() {
    // Nothing required
}

void BinaryReader::variant_begin(const std::span<const char*>& labels) {
    // Nothing required
}

bool BinaryReader::variant_match(const char* label) {
    const char* label_value = (const char*)&data[pos];
    std::size_t max_len = data.size() - pos;
    std::size_t len = strnlen((char*)&data[pos], max_len);
    if (len == max_len) {
        set_error("Unterminated string");
    }
    if (strncmp(label, label_value, max_len) == 0) {
        pos += (len + 1);
        return true;
    }
    return false;
}

std::tuple<const std::uint8_t*, std::size_t> BinaryReader::binary_data() {
    std::size_t size = 0;
    value_number(size);
    if (pos + size > data.size()) {
        set_error("Input data is too short");
    }
    const std::uint8_t* output_data = &data[pos];
    pos += size;
    return std::make_tuple(output_data, size);
}

void BinaryReader::trivial_begin(std::size_t size) {
    if (binary_depth == 0) {
        std::uint64_t binary_size = 0; // = 0 to avoid compiler warnings
        value_number<std::uint64_t>(binary_size);
        binary_start = pos;
        binary_end = binary_start + binary_size;
    }

    pad(size);
    binary_depth++;
}

void BinaryReader::trivial_end(std::size_t size) {
    pad(size);
    binary_depth--;
    if (binary_depth == 0 && pos != binary_end) {
        set_error("Didn't reach end of trivial as binary");
    }
}

void BinaryReader::object_begin() {
    // Do nothing
}

void BinaryReader::object_end() {
    // Do nothing
}

void BinaryReader::list_begin() {
    // Do nothing
}

void BinaryReader::list_end() {
    // Do nothing
}

bool BinaryReader::list_next() {
    if (binary_depth > 0) {
        if (binary_depth > 1) {
            set_error("Cannot have a list inside a trivial object");
            return false;
        }
        return pos != binary_end;
    }
    bool has_next;
    value_bool(has_next);
    return has_next;
}

} // namespace datapack
