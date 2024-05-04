#pragma once

#include <datapack/reader.h>

namespace datapack {

class RandomReader: public Reader {
public:
    void value_f64(double& value) override {
        value = (double)rand() / RAND_MAX;
    }
    void value_i32(int& value) override {
        value = rand() % 100;
    }

    bool optional_begin() override {
        return random() % 2 == 0;
    }
    void optional_end() override {

    }
    bool variant_begin(const char* label) override {
        // Note: Can't give a meaningful result here without the full schema
        return random() % 4;
    }
    void variant_end() override {

    }

    void object_begin() override {

    }
    void object_end() override {

    }
    void object_next(const char* key) override {

    }
};

} // namespace datapack
