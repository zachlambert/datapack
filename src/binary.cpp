#include "datapack/binary.h"


namespace datapack {

void BinaryReader::value_f64(double& value) {
    if (pos + sizeof(double) > data.size()) {
        error("Input data is too short");
        return;
    }
    value = *((double*)&data[pos]);
    pos += sizeof(double);
}

void BinaryReader::value_i32(int& value) {
    if (pos + sizeof(int) > data.size()) {
        error("Input data is too short");
        return;
    }
    value = *((int*)&data[pos]);
    pos += sizeof(int);
}

bool BinaryReader::optional_begin() {
    if (pos + 1 > data.size()) {
        error("Input data is too short");
    }
    std::uint8_t flag = data[pos];
    pos++;
    if (flag == 0x00) {
        return false;
    } else if (flag == 0x01) {
        return true;
    } else {
        error("Unexpected byte value for optional flag");
        return false;
    }
}

bool BinaryReader::variant_begin(const char* label) {
    bool match = std::strncmp(label, (char*)&data[pos], data.size()-pos) == 0;
    if (match) {
        pos += (std::strlen(label) + 1);
        return true;
    } else {
        return false;
    }
}

void BinaryWriter::value_f64(const double& value) {
    std::size_t pos = data.size();
    data.resize(data.size() + sizeof(double));
    *((double*)&data[pos]) = value;
}

void BinaryWriter::value_i32(const int& value) {
    std::size_t pos = data.size();
    data.resize(data.size() + sizeof(int));
    *((int*)&data[pos]) = value;
}

void BinaryWriter::optional_begin(bool has_value) {
    data.push_back(has_value ? 0x01 : 0x00);
}

void BinaryWriter::variant_begin(const char* label) {
    std::size_t length = strlen(label) + 1;
    std::size_t pos = data.size();
    data.resize(pos + length);
    strncpy((char*)&data[pos], label, data.size() - pos);
}

} // namespace datapack
