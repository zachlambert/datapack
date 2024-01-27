#pragma once

#include <sstream>
#include <stack>
#include "datapack/writer.h"

namespace datapack {

class YamlWriter: public Writer {
public:
    YamlWriter();

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

    std::string result();

private:
    void assert_is_array(bool expected);
    void assert_at_value(bool expected);
    void indent();

    std::stringstream ss;
    std::stack<bool> is_array;
    bool at_value;
    bool first_key_in_array;
};

} // namespace datpack
