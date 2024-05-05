#pragma once

#include "datapack/reader.hpp"
#include "datapack/writer.hpp"
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

    void value_f64(double& value) override;
    void value_i32(int& value) override;

    bool optional_begin() override;
    void optional_end() override {}

    bool variant_begin(const char* label) override;
    void variant_end() override {}

    void object_begin() override {}
    void object_end() override {}
    void object_next(const char* key) override {}

private:
    const std::vector<std::uint8_t>& data;
    std::size_t pos;
};

class BinaryWriter: public Writer {
public:
    BinaryWriter(std::vector<std::uint8_t>& data):
        data(data)
    {}

    void value_f64(const double& value) override;
    void value_i32(const int& value) override;

    void optional_begin(bool has_value) override;
    void optional_end() override {}
    void variant_begin(const char* label) override;
    void variant_end() override {}

    void object_begin() override {}
    void object_end() override {}
    void object_next(const char* key) override {}

private:
    std::vector<std::uint8_t>& data;
};

} // namespace datapack
