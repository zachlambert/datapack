#pragma once

#include "datapack/datapack.hpp"
#include "datapack/schema/token.hpp"

namespace datapack {

class Tokenizer : public Reader {
public:
  Tokenizer(std::vector<Token>& tokens);

  void number(NumberType type, void* value) override;
  bool boolean() override;
  const char* string() override;
  int enumerate(const std::span<const char*>& labels) override;
  std::span<const std::uint8_t> binary() override;

  bool optional_begin() override;
  void optional_end() override;

  int variant_begin(const std::span<const char*>& labels) override;
  void variant_tokenize(int index) override;
  void variant_end() override;

  void object_begin() override;
  void object_next(const char* key) override;
  void object_end() override;

  void tuple_begin() override;
  void tuple_next() override;
  void tuple_end() override;

  size_t list_begin() override;
  void list_next() override;
  void list_end() override;

  void hint(const Hint& hint) override;
  void description(const std::string& description) override;

private:
  std::vector<Token>& tokens;
  bool first_element;
};

} // namespace datapack
