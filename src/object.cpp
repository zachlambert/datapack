#include "datapack/object.h"

namespace datapack {

void ObjectWriter::i32(int value) {
    tokens.push_back(Primitive(value));
}

void ObjectWriter::i64(long value) {
    tokens.push_back(Primitive(value));
}

void ObjectWriter::f32(float value) {
    tokens.push_back(Primitive(value));
}

void ObjectWriter::f64(double value) {
    tokens.push_back(Primitive(value));
}

void ObjectWriter::string(const std::string& value) {
    tokens.push_back(Primitive(value));
}

void ObjectWriter::boolean(bool value) {
    tokens.push_back(Primitive(value));
}

void ObjectWriter::null() {
    tokens.push_back(Primitive(std::nullopt));
}

void ObjectWriter::binary(const binary_t& value) {
    tokens.push_back(Primitive(value));
}

void ObjectWriter::object_begin() {
    tokens.push_back(ObjectBegin());
}

void ObjectWriter::object_end() {
    tokens.push_back(ObjectEnd());
}

void ObjectWriter::object_element(const std::string& key) {
    tokens.push_back(ObjectElement(key));
}

void ObjectWriter::array_begin() {
    tokens.push_back(ArrayBegin());
}

void ObjectWriter::array_end() {
    tokens.push_back(ArrayEnd());
}

void ObjectWriter::array_element() {
    tokens.push_back(ArrayElement());
}

} // namespace datapack
