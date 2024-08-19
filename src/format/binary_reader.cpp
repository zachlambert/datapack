#include "datapack/format/binary_reader.hpp"
#include <assert.h>
#include <cstring>


namespace datapack {

const char* BinaryReader::value_string(const char*) {
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

bool BinaryReader::optional_begin(bool) {
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

std::tuple<const std::uint8_t*, std::size_t> BinaryReader::binary_data(
    std::size_t length,
    std::size_t stride)
{
    if (length == 0) {
        value_number(length);
    }
    std::size_t size = length * stride;
    if (pos + size > data.size()) {
        set_error("Input data is too short");
    }

    const std::uint8_t* output_data = &data[pos];
    pos += size;
    return std::make_tuple(output_data, length);
}

void BinaryReader::object_begin(std::size_t size) {
    if (size == 0) {
        return;
    }
    if (binary_depth == 0) {
        binary_start = pos;
    }
    pad(size);
    binary_depth++;
}

void BinaryReader::object_end(std::size_t size) {
    if (size == 0) {
        return;
    }
    pad(size);
    binary_depth--;
}

void BinaryReader::list_begin(bool is_trivial) {
    if (binary_depth != 0) {
        set_error("Cannot start a list inside a trivial list");
        return;
    }
    if (!is_trivial) {
        return;
    }
    std::uint64_t length = 0;
    value_number(length);
    trivial_list_remaining = length;

    binary_depth++;
    binary_start = pos;
}

void BinaryReader::list_end() {
    if (binary_depth == 0) {
        return;
    }
    binary_depth--;
    assert(binary_depth == 0);
}

ListNext BinaryReader::list_next(bool) {
    if (binary_depth > 0) {
        if (trivial_list_remaining == 0) {
            return ListNext::End;
        }
        trivial_list_remaining--;
        return ListNext::Next;
    }
    bool has_next;
    value_bool(has_next);
    return has_next ? ListNext::Next : ListNext::End;
}

} // namespace datapack
