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

void BinaryWriter::binary(std::size_t binary_size, const std::uint8_t* binary_data) {
    value_number((std::uint64_t)binary_size);
    std::size_t pos = data.size();
    data.resize(pos + binary_size);
    std::memcpy(&data[pos], binary_data, binary_size);
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


void BinaryWriter::list_begin() {
    if (is_writeable_binary()) {
        enforce_padding = true;
        container_start = data.size();
        container_size = 0;
        value_number(container_size);
    }
    // else, do nothing
}

void BinaryWriter::list_end() {
    if (enforce_padding) {
        enforce_padding = false;
        *((std::size_t*)&data[container_start]) = container_size;
    } else {
        value_bool(false);
    }
}

void BinaryWriter::list_next() {
    if (enforce_padding) {
        container_size++;
    } else {
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
    std::size_t max_len = data.size() - pos;
    std::size_t len = strnlen((char*)&data[pos], max_len);
    if (len == max_len) {
        error("Unterminated string");
    }
    next_variant_label = (char*)&data[pos];
    pos += (len + 1);
}

bool BinaryReader::variant_match(const char* label) {
    return strcmp(label, next_variant_label) == 0;
}

std::size_t BinaryReader::binary_size() {
    std::uint64_t size;
    value_number(size);
    next_binary_size = size;
    return size;
}

void BinaryReader::binary_data(std::uint8_t* output_data) {
    std::size_t size = next_binary_size;
    if (pos + size > data.size()) {
        error("Input data is too short");
    }
    std::memcpy(output_data, &data[pos], size);
    pos += size;
}


void BinaryReader::map_begin() {
    // Do nothing
}

void BinaryReader::map_end() {
    // Do nothing
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


void BinaryReader::list_begin() {
    // Do nothing
}

void BinaryReader::list_end() {
    // Do nothing
}

bool BinaryReader::list_next() {
    bool has_next;
    value_bool(has_next);
    return has_next;
}


} // namespace datapack
