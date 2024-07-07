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

void BinaryWriter::object_begin() {
    if (is_array_) {
        binary_blocks.push_back(BinaryBlock(pos));
    }
}

void BinaryWriter::object_end() {
    if (is_array_) {
        const auto& top = binary_blocks.back();
        while ((pos - top.start) % top.padding != 0) {
            if (!resize(pos + 1)) {
                return;
            }
            data[pos] = 0x00;
            pos++;
        }
        std::size_t size = pos - top.start;
        binary_blocks.pop_back();
        if (!binary_blocks.empty()) {
            binary_blocks.back().padding = std::max(binary_blocks.back().padding, size);
        } else {
            binary_size += size;
        }
    }
}

void BinaryWriter::list_begin(bool is_array) {
    if (is_array_) {
        object_begin();
        return;
    }
    if (is_array) {
        value_number<std::uint64_t>(0); // Placeholder
        is_array_ = true;
        binary_size = 0;
    }
}

void BinaryWriter::list_end() {
    if (is_array_) {
        if (!binary_blocks.empty()) {
            object_end();
            return;
        }
        is_array_ = false;
        std::size_t start = pos - binary_size - sizeof(std::uint64_t);
        *((std::uint64_t*)&data[start]) = binary_size;
        return;
    }
    value_bool(false);
}

void BinaryWriter::list_next() {
    if (!is_array_) {
        value_bool(true);
    }
}


} // namespace datapack
