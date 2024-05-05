#pragma once

#include "datapack/writer.h"
#include <iostream>

namespace datapack {

class DebugWriter: public Writer {
public:
    DebugWriter(std::ostream& os):
        os(os),
        depth(0)
    {}

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
    void indent();

    std::ostream& os;
    int depth;
};

} // namespace datapack
