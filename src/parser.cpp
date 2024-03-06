#include "datapack/parser.hpp"

namespace datapack {

Object parse(Parser& parser) {
    Object object;
    while (true) {
        auto next = parser.next();
        if (!next.has_value()) {
            break;
        }
        object.tokens.push_back(next.value());
    }
    return object;
}

Object parse(Parser&& parser) {
    return parse(parser);
}

ParserReader::ParserReader(Parser& parser):
    parser(parser)
{}

int ParserReader::i32() {
    try {
        return std::get<int>(std::get<Primitive>(parser.next().value()));
    } catch (const std::bad_optional_access&) {
        throw ReadException("Incorrect token");
    }
}

long ParserReader::i64() {
    try {
        return std::get<long>(std::get<Primitive>(parser.next().value()));
    } catch (const std::bad_optional_access&) {
        throw ReadException("Incorrect token");
    }
}

float ParserReader::f32() {
    try {
        return std::get<float>(std::get<Primitive>(parser.next().value()));
    } catch (const std::bad_optional_access&) {
        throw ReadException("Incorrect token");
    }
}

double ParserReader::f64() {
    try {
        return std::get<double>(std::get<Primitive>(parser.next().value()));
    } catch (const std::bad_optional_access&) {
        throw ReadException("Incorrect token");
    }
}

std::string ParserReader::string() {
    try {
        return std::get<std::string>(std::get<Primitive>(parser.next().value()));
    } catch (const std::bad_optional_access&) {
        throw ReadException("Incorrect token");
    }
}

bool ParserReader::boolean() {
    try {
        return std::get<bool>(std::get<Primitive>(parser.next().value()));
    } catch (const std::bad_optional_access&) {
        throw ReadException("Incorrect token");
    }
}

bool ParserReader::null() {
    try {
        auto token = parser.next().value();
        if (std::get_if<std::nullopt_t>(&std::get<Primitive>(token))) {
            return true;
        }
        return false;
    } catch (const std::bad_optional_access&) {
        throw ReadException("Incorrect token");
    }
    return false; // Unreachable
}

binary_t ParserReader::binary() {
    try {
        return std::get<binary_t>(std::get<Primitive>(parser.next().value()));
    } catch (const std::bad_optional_access&) {
        throw ReadException("Incorrect token");
    }
}


void ParserReader::object_begin() {
    if (pos >= object.tokens.size()) {
        throw ReadException("Reached end of input");
    }
    if (!std::get_if<ObjectBegin>(&object.tokens[pos])) {
        throw ReadException("Incorrect token");
    }
    pos++;
}

void ParserReader::object_end() {
    if (pos >= object.tokens.size()) {
        throw ReadException("Reached end of input");
    }
    if (!std::get_if<ObjectEnd>(&object.tokens[pos])) {
        throw ReadException("Incorrect token");
    }
    pos++;
}

void ParserReader::object_element(const std::string& key) {
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


void ParserReader::array_begin() {
    if (pos >= object.tokens.size()) {
        throw ReadException("Reached end of input");
    }
    if (!std::get_if<ArrayBegin>(&object.tokens[pos])) {
        throw ReadException("Incorrect token");
    }
    pos++;
}

void ParserReader::array_end() {
    if (pos >= object.tokens.size()) {
        throw ReadException("Reached end of input");
    }
    if (!std::get_if<ArrayEnd>(&object.tokens[pos])) {
        throw ReadException("Incorrect token");
    }
    pos++;
}

bool ParserReader::array_element() {
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
