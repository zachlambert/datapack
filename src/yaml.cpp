#include "datapack/yaml.h"

namespace datapack {

YamlWriter::YamlWriter():
    at_value(true),
    first_key_in_array(false)
{
    // start_object();
}

YamlWriter& YamlWriter::key(const std::string& key) {
    assert_at_value(false);
    assert_is_array(false);
    if (!first_key_in_array) {
        indent();
    }
    first_key_in_array = false;
    ss << key << ": ";
    at_value = true;
    return *this;
}

YamlWriter& YamlWriter::next() {
    assert_at_value(false);
    assert_is_array(true);
    indent();
    ss << "- ";
    at_value = true;
    return *this;
}

YamlWriter& YamlWriter::i32(int value) {
    assert_at_value(true);
    ss << value << "\n";
    at_value = false;
    return *this;
}

YamlWriter& YamlWriter::i64(long value) {
    assert_at_value(true);
    ss << value << "\n";
    at_value = false;
    return *this;
}

YamlWriter& YamlWriter::f32(float value) {
    assert_at_value(true);
    ss << value << "\n";
    at_value = false;
    return *this;
}

YamlWriter& YamlWriter::f64(double value) {
    assert_at_value(true);
    ss << value << "\n";
    at_value = false;
    return *this;
}

YamlWriter& YamlWriter::string(const std::string& value) {
    assert_at_value(true);
    ss << value << "\n";
    at_value = false;
    return *this;
}

YamlWriter& YamlWriter::boolean(bool value) {
    assert_at_value(true);
    ss << (value ? "true" : "false") << "\n";
    at_value = false;
    return *this;
}

YamlWriter& YamlWriter::null() {
    assert_at_value(true);
    ss << "null\n";
    at_value = false;
    return *this;
}

YamlWriter& YamlWriter::binary(const binary_t& value) {
    assert_at_value(true);
    binary_t encoded;
    base64_encode(value, encoded);
    for (auto x: encoded) {
        ss << x;
    }
    ss << "\n";
    at_value = false;
    return *this;
}

YamlWriter& YamlWriter::start_array() {
    ss << "\n";
    assert_at_value(true);
    is_array.push(true);
    at_value = false;
    return *this;
}

YamlWriter& YamlWriter::end_array() {
    assert_is_array(true);
    if (is_array.empty()) {
        throw WriteException("Unknown error");
    }
    is_array.pop();
    return *this;
}

YamlWriter& YamlWriter::start_object() {
    if (!is_array.empty()) {
        if (is_array.top()) {
            first_key_in_array = true;
        } else {
            ss << "\n";
        }
    }
    assert_at_value(true);
    is_array.push(false);
    at_value = false;
    return *this;
}

YamlWriter& YamlWriter::end_object() {
    assert_is_array(false);
    if (is_array.empty()) {
        throw WriteException("Unknown error");
    }
    is_array.pop();
    return *this;
}

std::string YamlWriter::result() {
    if (!is_array.empty()) {
        throw WriteException("A container hasn't ended");
    }
    std::string result =  ss.str();
    ss.str("");
    return result;
}

void YamlWriter::assert_is_array(bool expected) {
    if (expected && !is_array.top()) {
        throw WriteException("Must be in an array");
    }
    if (!expected && is_array.top()) {
        throw WriteException("Must be in an object");
    }
}

void YamlWriter::assert_at_value(bool expected) {
    if (expected && !at_value) {
        throw WriteException("Cannot write a value here");
    }
    if (!expected && at_value) {
        throw WriteException("Expecting a value here");
    }
}

void YamlWriter::indent() {
    // Indent size must be 2
    const int indent_size = 2;
    const int levels = is_array.size() - 1;
    for (std::size_t i = 0; i < levels * indent_size; i++) {
        ss << " ";
    }
}

} // namespace datapack
