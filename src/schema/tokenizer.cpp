#include "datapack/schema/tokenizer.hpp"

namespace datapack {

Tokenizer::Tokenizer(std::vector<Token>& tokens):
    Reader(false, false, true),
    tokens(tokens),
    first_element(false)
{
    tokens.clear();
}

void Tokenizer::value_i32(std::int32_t& value) {
    tokens.push_back(value);
}

void Tokenizer::value_i64(std::int64_t& value) {
    tokens.push_back(value);
}

void Tokenizer::value_u32(std::uint32_t& value) {
    tokens.push_back(value);
}

void Tokenizer::value_u64(std::uint64_t& value) {
    tokens.push_back(value);
}


void Tokenizer::value_f32(float& value) {
    tokens.push_back(value);
}

void Tokenizer::value_f64(double& value) {
    tokens.push_back(value);
}


const char* Tokenizer::value_string() {
    tokens.push_back(std::string());
    return nullptr;
}

void Tokenizer::value_bool(bool& value) {
    tokens.push_back(value);
}


int Tokenizer::enumerate(const std::span<const char*>& labels) {
    tokens.push_back(token::Enumerate(labels));
    return 0;
}

bool Tokenizer::optional_begin() {
    tokens.push_back(token::Optional());
    return true;
}

void Tokenizer::optional_end() {
    // Nothing required
}

void Tokenizer::variant_begin(const std::span<const char*>& labels) {
    tokens.push_back(token::VariantBegin(labels));
}

bool Tokenizer::variant_match(const char* label) {
    tokens.push_back(token::VariantNext(label));
    return true;
}

void Tokenizer::variant_end() {
    tokens.push_back(token::VariantEnd());
}


std::tuple<const std::uint8_t*, std::size_t> Tokenizer::binary_data() {
    tokens.push_back(token::BinaryData());
    return std::make_tuple(nullptr, 0);
}

void Tokenizer::object_begin() {
    tokens.push_back(token::ObjectBegin());
}

void Tokenizer::object_end() {
    tokens.push_back(token::ObjectEnd());
}

void Tokenizer::object_next(const char* key) {
    tokens.push_back(token::ObjectNext(key));
}


void Tokenizer::tuple_begin() {
    tokens.push_back(token::TupleBegin());
}

void Tokenizer::tuple_end() {
    tokens.push_back(token::TupleEnd());
}

void Tokenizer::tuple_next() {
    tokens.push_back(token::TupleNext());
}


void Tokenizer::list_begin(bool is_array) {
    tokens.push_back(token::List(is_array));
    first_element = true;
}

void Tokenizer::list_end() {
    // Do nothing
}

bool Tokenizer::list_next() {
    if (first_element) {
        first_element = false;
        return true;
    }
    return false;
}

} // namespace datapack
