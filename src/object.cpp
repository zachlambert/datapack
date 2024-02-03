#include "datapack/object.hpp"

namespace datapack {

void write(Writer& writer, const Object& object) {
    for (const auto& token: object.tokens) {
        if (std::get_if<ObjectBegin>(&token)) {
            writer.object_begin();
        }
        else if (std::get_if<ObjectEnd>(&token)) {
            writer.object_end();
        }
        else if (auto value = std::get_if<ObjectElement>(&token)) {
            writer.object_element(value->key);
        }
        else if (std::get_if<ArrayBegin>(&token)) {
            writer.array_begin();
        }
        else if (std::get_if<ArrayEnd>(&token)) {
            writer.array_end();
        }
        else if (std::get_if<ArrayElement>(&token)) {
            writer.array_element();
        }
        else if (auto value = std::get_if<Primitive>(&token)) {
            std::visit([&writer](const auto& value) {
                writer.value(value);
            }, *value);
        }
        else {
            throw WriteException("Unreachable code");
        }
    }
}

void ObjectWriter::i32(int value) {
    object.tokens.push_back(Primitive(value));
}

void ObjectWriter::i64(long value) {
    object.tokens.push_back(Primitive(value));
}

void ObjectWriter::f32(float value) {
    object.tokens.push_back(Primitive(value));
}

void ObjectWriter::f64(double value) {
    object.tokens.push_back(Primitive(value));
}

void ObjectWriter::string(const std::string& value) {
    object.tokens.push_back(Primitive(value));
}

void ObjectWriter::boolean(bool value) {
    object.tokens.push_back(Primitive(value));
}

void ObjectWriter::null() {
    object.tokens.push_back(Primitive(std::nullopt));
}

void ObjectWriter::binary(const binary_t& value) {
    object.tokens.push_back(Primitive(value));
}

void ObjectWriter::object_begin() {
    object.tokens.push_back(ObjectBegin());
}

void ObjectWriter::object_end() {
    object.tokens.push_back(ObjectEnd());
}

void ObjectWriter::object_element(const std::string& key) {
    object.tokens.push_back(ObjectElement(key));
}

void ObjectWriter::array_begin() {
    object.tokens.push_back(ArrayBegin());
}

void ObjectWriter::array_end() {
    object.tokens.push_back(ArrayEnd());
}

void ObjectWriter::array_element() {
    object.tokens.push_back(ArrayElement());
}


ObjectReader::ObjectReader(const Object& object):
    object(object),
    pos(0)
{}

void ObjectReader::i32(int& value) {
    if (pos >= object.tokens.size()) {
        throw ReadException("Reached end of input");
    }
    try {
        value = std::get<int>(std::get<Primitive>(object.tokens[pos]));
    } catch (const std::bad_optional_access&) {
        throw ReadException("Incorrect token");
    }
    pos++;
}

void ObjectReader::i64(long& value) {
    if (pos >= object.tokens.size()) {
        throw ReadException("Reached end of input");
    }
    try {
        value = std::get<long>(std::get<Primitive>(object.tokens[pos]));
    } catch (const std::bad_optional_access&) {
        throw ReadException("Incorrect token");
    }
    pos++;
}

void ObjectReader::f32(float& value) {
    if (pos >= object.tokens.size()) {
        throw ReadException("Reached end of input");
    }
    try {
        value = std::get<float>(std::get<Primitive>(object.tokens[pos]));
    } catch (const std::bad_optional_access&) {
        throw ReadException("Incorrect token");
    }
    pos++;
}

void ObjectReader::f64(double& value) {
    if (pos >= object.tokens.size()) {
        throw ReadException("Reached end of input");
    }
    try {
        value = std::get<double>(std::get<Primitive>(object.tokens[pos]));
    } catch (const std::bad_optional_access&) {
        throw ReadException("Incorrect token");
    }
    pos++;
}

void ObjectReader::string(std::string& value) {
    if (pos >= object.tokens.size()) {
        throw ReadException("Reached end of input");
    }
    try {
        value = std::get<std::string>(std::get<Primitive>(object.tokens[pos]));
    } catch (const std::bad_optional_access&) {
        throw ReadException("Incorrect token");
    }
    pos++;
}

void ObjectReader::boolean(bool& value) {
    if (pos >= object.tokens.size()) {
        throw ReadException("Reached end of input");
    }
    try {
        value = std::get<bool>(std::get<Primitive>(object.tokens[pos]));
    } catch (const std::bad_optional_access&) {
        throw ReadException("Incorrect token");
    }
    pos++;
}

bool ObjectReader::null() {
    if (pos >= object.tokens.size()) {
        throw ReadException("Reached end of input");
    }
    try {
        if (std::get_if<std::nullopt_t>(&std::get<Primitive>(object.tokens[pos]))) {
            pos++;
            return true;
        }
        return false;
    } catch (const std::bad_optional_access&) {
        throw ReadException("Incorrect token");
    }
    return false; // Unreachable
}

void ObjectReader::binary(binary_t& value) {
    if (pos >= object.tokens.size()) {
        throw ReadException("Reached end of input");
    }
    try {
        value = std::get<binary_t>(std::get<Primitive>(object.tokens[pos]));
    } catch (const std::bad_optional_access&) {
        throw ReadException("Incorrect token");
    }
    pos++;
}


void ObjectReader::object_begin() {
    if (pos >= object.tokens.size()) {
        throw ReadException("Reached end of input");
    }
    if (!std::get_if<ObjectBegin>(&object.tokens[pos])) {
        throw ReadException("Incorrect token");
    }
    pos++;
}

void ObjectReader::object_end() {
    if (pos >= object.tokens.size()) {
        throw ReadException("Reached end of input");
    }
    if (!std::get_if<ObjectEnd>(&object.tokens[pos])) {
        throw ReadException("Incorrect token");
    }
    pos++;
}

void ObjectReader::object_element(const std::string& key) {
    if (pos >= object.tokens.size()) {
        throw ReadException("Reached end of input");
    }
    try {
        if (key != std::get<ObjectElement>(object.tokens[pos]).key) {
            throw ReadException("Incorrect key");
        }
    } catch (const std::bad_optional_access&) {
        throw ReadException("Incorrect token");
    }
    pos++;
}


void ObjectReader::array_begin() {
    if (pos >= object.tokens.size()) {
        throw ReadException("Reached end of input");
    }
    if (!std::get_if<ArrayBegin>(&object.tokens[pos])) {
        throw ReadException("Incorrect token");
    }
    pos++;
}

void ObjectReader::array_end() {
    if (pos >= object.tokens.size()) {
        throw ReadException("Reached end of input");
    }
    if (!std::get_if<ArrayEnd>(&object.tokens[pos])) {
        throw ReadException("Incorrect token");
    }
    pos++;
}

bool ObjectReader::array_element() {
    if (pos >= object.tokens.size()) {
        throw ReadException("Reached end of input");
    }
    if (!std::get_if<ArrayElement>(&object.tokens[pos])) {
        return false;
    }
    pos++;
    return true;
}

} // namespace datapack
