#include "datapack/format/binary_reader.hpp"

#include <cstring>

namespace datapack {

void BinaryReader::value_string(std::string& value) {
    std::size_t max_len = data.size() - pos;
    std::size_t len = strnlen((char*)&data[pos], max_len);
    if (len == max_len) {
        error("Unterminated string");
    }
    value = (char*)&data[pos];
    pos += (len + 1);
}

void BinaryReader::value_bool(bool& value) {
    if (pos + 1 > data.size()) {
        error("Input data too short");
    }
    std::uint8_t byte = data[pos];
    pos++;
    if (byte >= 0x02) {
        error("Unexpected byte for bool");
    }
    value = (byte == 0x01);
}


int BinaryReader::enumerate(const std::vector<const char*>& labels) {
    int value;
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

void BinaryReader::variant_begin(const std::vector<const char*>& labels) {
    // Nothing required
}

bool BinaryReader::variant_match(const char* label) {
    const char* label_value = (const char*)&data[pos];
    std::size_t max_len = data.size() - pos;
    std::size_t len = strnlen((char*)&data[pos], max_len);
    if (len == max_len) {
        error("Unterminated string");
    }
    if (strncmp(label, label_value, max_len) == 0) {
        pos += (len + 1);
        return true;
    }
    return false;
}

std::tuple<const std::uint8_t*, std::size_t> BinaryReader::binary_data() {
    std::size_t size;
    value_number(size);
    if (pos + size > data.size()) {
        error("Input data is too short");
    }
    const std::uint8_t* output_data = &data[pos];
    pos += size;
    return std::make_tuple(output_data, size);
}

void BinaryReader::object_begin() {
    if (is_array_) {
        binary_blocks.push_back(BinaryBlock(pos));
    }
}

void BinaryReader::object_end() {
    if (is_array_) {
        const auto& top = binary_blocks.back();
        while ((pos - top.start) % top.padding != 0) {
            pos++;
        }
        std::size_t size = pos - top.start;
        binary_blocks.pop_back();
        if (!binary_blocks.empty()) {
            binary_blocks.back().padding = std::max(binary_blocks.back().padding, size);
        } else {
            binary_remaining -= size;
        }
    }
}

bool BinaryReader::map_next(std::string& key) {
    bool has_next;
    value_bool(has_next);
    if (!has_next) {
        return false;
    }
    value_string(key);
    return true;
}

void BinaryReader::list_begin(bool is_array) {
    if (is_array_) {
        object_begin();
        return;
    }
    if (is_array) {
        std::uint64_t size;
        value_number<std::uint64_t>(size);
        is_array_ = true;
        binary_remaining = size;
    }
}

void BinaryReader::list_end() {
    if (is_array_) {
        if (!binary_blocks.empty()) {
            object_end();
        } else {
            is_array_ = false;
        }
    }
}

bool BinaryReader::list_next() {
    if (is_array_) {
        if (!binary_blocks.empty()) {
            // If here, then the caller is responsible for calling list_next()
            // the correct number of times, unless a Reader returns true
            // to stop early.
            return true;
        }
        return binary_remaining > 0;
    }
    bool has_next;
    value_bool(has_next);
    return has_next;
}

} // namespace datapack
