#include "datapack/format/binary.hpp"


namespace datapack {

void BinaryWriter::value_string(const std::string& value) {
    std::size_t size = value.size() + 1;
    std::size_t pos = data.size();
    data.resize(data.size() + size);
    strncpy((char*)&data[pos], value.c_str(), size);
}

void BinaryWriter::value_bool(bool value) {
    data.push_back(value ? 0x01 : 0x00);
}


void BinaryWriter::enumerate(int value, const std::vector<const char*>& labels) {
    value_number(value);
}

void BinaryWriter::optional(bool has_value) {
    value_bool(has_value);
}

void BinaryWriter::variant_begin(const char* label, const std::vector<const char*>& labels) {
    value_string(label);
}

void BinaryWriter::binary_data(const std::uint8_t* input_data, std::size_t size) {
    value_number(std::uint64_t(size));
    std::size_t pos = data.size();
    data.resize(pos + size);
    std::memcpy(&data[pos], input_data, size);
}

void BinaryWriter::object_begin() {
    if (is_array_) {
        binary_blocks.push(BinaryBlock(data.size()));
    }
}

void BinaryWriter::object_end() {
    if (is_array_) {
        const auto& top = binary_blocks.top();
        while ((data.size() - top.start) % top.padding != 0) {
            data.push_back(0x00);
        }
        std::size_t size = data.size() - top.start;
        binary_blocks.pop();
        if (!binary_blocks.empty()) {
            binary_blocks.top().padding = std::max(binary_blocks.top().padding, size);
        } else {
            binary_size += size;
        }
    }
}

void BinaryWriter::map_begin() {

}

void BinaryWriter::map_end() {
    value_bool(false);
}

void BinaryWriter::map_next(const std::string& key) {
    value_bool(true);
    value_string(key);
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
        std::size_t start = data.size() - binary_size - sizeof(std::uint64_t);
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

bool BinaryReader::optional() {
    bool has_value;
    value_bool(has_value);
    return has_value;
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
        binary_blocks.push(BinaryBlock(pos));
    }
}

void BinaryReader::object_end() {
    if (is_array_) {
        const auto& top = binary_blocks.top();
        while ((pos - top.start) % top.padding != 0) {
            pos++;
        }
        std::size_t size = pos - top.start;
        binary_blocks.pop();
        if (!binary_blocks.empty()) {
            binary_blocks.top().padding = std::max(binary_blocks.top().padding, size);
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
            return false; // Unused
        }
        return binary_remaining > 0;
    }
    bool has_next;
    value_bool(has_next);
    return has_next;
}


} // namespace datapack
