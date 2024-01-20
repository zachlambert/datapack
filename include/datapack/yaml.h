#pragma once

#include <sstream>
#include <stack>
#include "datapack/writer.h"

namespace datapack {

class YamlWriter: public Writer {
public:
    YamlWriter();

    YamlWriter& key(const std::string& key) override;
    YamlWriter& next() override;

    YamlWriter& i32(int value) override;
    YamlWriter& i64(long value) override;
    YamlWriter& f32(float value) override;
    YamlWriter& f64(double value) override;
    YamlWriter& string(const std::string& value) override;
    YamlWriter& boolean(bool value) override;
    YamlWriter& null() override;
    YamlWriter& binary(const binary_t& value) override;

    template <writeable T>
    YamlWriter& value(const T& value) {
        write(*this, value);
        return *this;
    }

    YamlWriter& start_array() override;
    YamlWriter& end_array() override;

    YamlWriter& start_object() override;
    YamlWriter& end_object() override;

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
