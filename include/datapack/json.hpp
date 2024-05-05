#pragma once

#include "datapack/parser.hpp"
#include "datapack/writer.hpp"

namespace datapack {

class JsonParser {
public:
};

class JsonWriter: public Writer {
public:
    JsonWriter(std::string& data):
        data(data)
    {
        data.clear();
    }

    void value_f64(const double& value) override;
    void value_i32(const int& value) override;

    void optional_begin(bool has_value) override;
    void optional_end() override;
    void variant_begin(const char* label) override;
    void variant_end() override;

    void object_begin() override;
    void object_end() override;
    void object_next(const char* key) override;

private:
    std::string& data;
};

} // namespace datapack
