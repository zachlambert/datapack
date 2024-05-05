#pragma once

#include "datapack/reader.h"
#include "datapack/writer.h"
#include <vector>
#include <cstring>
#include <iostream>


namespace datapack {

class BinaryReader: public Reader {
public:
    BinaryReader(const std::vector<std::uint8_t>& data):
        data(data),
        pos(0)
    {}

    void value_f64(double& value) override {
        if (pos + sizeof(double) > data.size()) {
            error("Input data is too short");
            return;
        }
        value = *((double*)&data[pos]);
        pos += sizeof(double);
    }

    void value_i32(int& value) override {
        if (pos + sizeof(int) > data.size()) {
            error("Input data is too short");
            return;
        }
        value = *((int*)&data[pos]);
        pos += sizeof(int);
    }

    bool optional_begin() override {
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

    void optional_end() override {

    }

    bool variant_begin(const char* label) override {
        bool match = std::strncmp(label, (char*)&data[pos], data.size()-pos) == 0;
        if (match) {
            pos += (std::strlen(label) + 1);
            return true;
        } else {
            return false;
        }
    }

    void variant_end() override {

    }

    void object_begin() override {

    }
    void object_end() override {

    }
    void object_next(const char* key) override {

    }

private:
    const std::vector<std::uint8_t>& data;
    std::size_t pos;
};

class BinaryWriter: public Writer {
public:
    BinaryWriter(std::vector<std::uint8_t>& data):
        data(data)
    {}

    void value_f64(const double& value) override {
        std::size_t pos = data.size();
        data.resize(data.size() + sizeof(double));
        *((double*)&data[pos]) = value;
    }

    void value_i32(const int& value) override {
        std::size_t pos = data.size();
        data.resize(data.size() + sizeof(int));
        *((int*)&data[pos]) = value;
    }

    void optional_begin(bool has_value) override {
        data.push_back(has_value ? 0x01 : 0x00);
    }
    void optional_end() override {

    }
    void variant_begin(const char* label) override {
        std::size_t length = strlen(label) + 1;
        std::size_t pos = data.size();
        data.resize(pos + length);
        strncpy((char*)&data[pos], label, data.size() - pos);
    }
    void variant_end() override {

    }

    void object_begin() override {

    }
    void object_end() override {

    }
    void object_next(const char* key) override {

    }

private:
    std::vector<std::uint8_t>& data;
};

} // namespace datapack
