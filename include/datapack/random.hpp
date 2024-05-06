#pragma once

#include <datapack/reader.hpp>

namespace datapack {

class RandomReader: public Reader {
public:
    RandomReader();

    void value_f64(double& value) override;
    void value_i32(int& value) override;

    bool optional_begin() override;
    void optional_end() override;
    void variant_begin(const std::vector<std::string>& types) override;
    bool variant_match(const char* label) override;
    void variant_end() override;

    void object_begin() override;
    void object_end() override;
    void object_next(const char* key) override;

private:
    std::string next_variant;
};

} // namespace datapack
