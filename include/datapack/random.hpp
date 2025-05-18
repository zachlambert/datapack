#pragma once

#include "datapack/datapack.hpp"
#include <cstdint>
#include <stack>
#include <string>
#include <tuple>
#include <vector>

namespace datapack {

class RandomReader : public Reader {
public:
  RandomReader();

  void number(NumberType type, void* value) override;
  bool boolean() override;
  const char* string() override;
  int enumerate(const std::span<const char*>& labels) override;
  std::span<const std::uint8_t> binary() override;

  bool optional_begin() override;
  void optional_end() override {}

  int variant_begin(const std::span<const char*>& labels) override;
  void variant_end() override {}

  void object_begin() override {}
  void object_end() override {}
  void object_next(const char* key) override {}

  void tuple_begin() override {}
  void tuple_end() override {}
  void tuple_next() override {}

  void list_begin() override;
  bool list_next() override;
  void list_end() override;

private:
  std::vector<std::uint8_t> data_temp;
  std::stack<int> list_counters;
  std::string string_temp;
};

template <readable T>
T random() {
  T result;
  RandomReader().value(result);
  return result;
}

} // namespace datapack
