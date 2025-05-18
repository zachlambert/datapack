#include "datapack/schema/tokenizer.hpp"

namespace datapack {

Tokenizer::Tokenizer(std::vector<Token>& tokens) :
    Reader(true), tokens(tokens), first_element(false) {
  tokens.clear();
}

void Tokenizer::number(NumberType type, void* value) { tokens.push_back(type); }

bool Tokenizer::boolean() {
  tokens.push_back(bool());
  return false;
}

const char* Tokenizer::string() {
  tokens.push_back(std::string());
  return nullptr;
}

int Tokenizer::enumerate(const std::span<const char*>& labels) {
  tokens.push_back(token::Enumerate(labels));
  return 0;
}

std::span<const std::uint8_t> Tokenizer::binary() {
  tokens.push_back(token::Binary());
  return std::span<const std::uint8_t>((const std::uint8_t*)nullptr, 0);
}

bool Tokenizer::optional_begin() {
  tokens.push_back(token::Optional());
  return true;
}

void Tokenizer::optional_end() {
  // Nothing required
}

int Tokenizer::variant_begin(const std::span<const char*>& labels) {
  tokens.push_back(token::VariantBegin(labels));
  return 0;
}

void Tokenizer::variant_tokenize(int index) { tokens.push_back(token::VariantNext(index)); }

void Tokenizer::variant_end() { tokens.push_back(token::VariantEnd()); }

void Tokenizer::object_begin() { tokens.push_back(token::ObjectBegin()); }

void Tokenizer::object_next(const char* key) { tokens.push_back(token::ObjectNext(key)); }

void Tokenizer::object_end() { tokens.push_back(token::ObjectEnd()); }

void Tokenizer::tuple_begin() { tokens.push_back(token::TupleBegin()); }

void Tokenizer::tuple_next() { tokens.push_back(token::TupleNext()); }

void Tokenizer::tuple_end() { tokens.push_back(token::TupleEnd()); }

void Tokenizer::list_begin() {
  tokens.push_back(token::List());
  first_element = true;
}

bool Tokenizer::list_next() {
  // No need to maintain stack, list_next() will be the first thing
  // called after list_begin() so only need to check if this is the first
  // call after list_begin()
  if (first_element) {
    first_element = false;
    return true;
  }
  return false;
}

void Tokenizer::list_end() {
  // Nothing required
}

} // namespace datapack
