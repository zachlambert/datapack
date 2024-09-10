#include "datapack/schema/tokenizer.hpp"

namespace datapack {

Tokenizer::Tokenizer(std::vector<Token>& tokens):
    Editor(true),
    tokens(tokens),
    first_element(false)
{
    tokens.clear();
}

void Tokenizer::primitive(Primitive primitive, void* value) {
    tokens.push_back(primitive);
}

const char* Tokenizer::string(const char*) {
    tokens.push_back(std::string());
    return nullptr;
}

int Tokenizer::enumerate(int, const std::span<const char*>& labels) {
    tokens.push_back(token::Enumerate(labels));
    return 0;
}

bool Tokenizer::optional_begin(bool) {
    tokens.push_back(token::Optional());
    return true;
}

void Tokenizer::optional_end() {
    // Nothing required
}

int Tokenizer::variant_begin(int value, const std::span<const char*>& labels) {
    tokens.push_back(token::VariantBegin(labels));
    return value;
}

void Tokenizer::variant_tokenize(int index) {
    tokens.push_back(token::VariantNext(index));
}

void Tokenizer::variant_end() {
    tokens.push_back(token::VariantEnd());
}


void Tokenizer::binary_data(std::uint8_t* data, std::size_t length, std::size_t stride, bool fixed_length) {
    tokens.push_back(token::BinaryData(length, stride, fixed_length));
}

void Tokenizer::object_begin(std::size_t size) {
    tokens.push_back(token::ObjectBegin(size));
}

void Tokenizer::object_end(std::size_t size) {
    tokens.push_back(token::ObjectEnd(size));
}

void Tokenizer::object_next(const char* key) {
    tokens.push_back(token::ObjectNext(key));
}


void Tokenizer::tuple_begin(std::size_t size) {
    tokens.push_back(token::TupleBegin(size));
}

void Tokenizer::tuple_end(std::size_t size) {
    tokens.push_back(token::TupleEnd(size));
}

void Tokenizer::tuple_next() {
    tokens.push_back(token::TupleNext());
}


void Tokenizer::list_begin(bool is_trivial) {
    tokens.push_back(token::List(is_trivial));
    first_element = true;
}

ContainerAction Tokenizer::list_end() {
    return ContainerAction::None;
}

void Tokenizer::list_next() {

}

} // namespace datapack
