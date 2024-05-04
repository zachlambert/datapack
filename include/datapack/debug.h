#pragma once

#include "datapack/writer.h"
#include <iostream>

namespace datapack {

class DebugWriter: public Writer {
public:
    DebugWriter(std::ostream& os):
        os(os)
    {}

    void value_f64(const double& value) override {
        os << value << "\n";
    }

    void value_i32(const int& value) override {
        os << value << "\n";
    }

    void optional_begin(bool has_value) override {
        if (!has_value) {
            os << "null\n";
        }
    }
    void optional_end() override {

    }
    void variant_begin(const char* label) override {
        os << "value[" << label << "]: ";
    }
    void variant_end() override {

    }

    void object_begin() override {
        os << "{\n";
    }
    void object_end() override {
        os << "},\n";
    }
    void object_next(const char* key) override {
        os << "\"" << key << "\": ";
    }

private:
    std::ostream& os;
};

} // namespace datapack
