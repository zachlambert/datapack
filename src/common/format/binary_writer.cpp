#include "datapack/format/binary_writer.hpp"


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

void BinaryWriter::optional_begin(bool has_value) {
    value_bool(has_value);
}

void BinaryWriter::optional_end() {
    // Nothing required
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


} // namespace datapack
