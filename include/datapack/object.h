#pragma once

#include <memory>
#include <variant>
#include <optional>
#include <vector>

#include "datapack/token.h"
#include "datapack/writer.h"
#include "datapack/reader.h"

namespace datapack {

class ObjectWriter: public Writer {
public:
    void i32(int value) override;
    void i64(long value) override;
    void f32(float value) override;
    void f64(double value) override;
    void string(const std::string& value) override;
    void boolean(bool value) override;
    void null() override;
    void binary(const binary_t& value) override;

    void object_begin() override;
    void object_end() override;
    void object_element(const std::string& key) override;

    void array_begin() override;
    void array_end() override;
    void array_element() override;

    const std::vector<Token>& result() {
        return tokens;
    }

private:
    std::vector<Token> tokens;
};

} // namespace datapack
