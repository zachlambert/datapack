#pragma once

#include <datapack/reader.hpp>
#include <tuple>
#include <stack>
#include <string>
#include <vector>
#include <cstdint>

namespace datapack {

class RandomReader : public Reader {
public:
  RandomReader();

  void integer(IntType type, void* value) override;
  void floating(FloatType type, void* value) override;
  bool boolean() override;
  const char* string() override;
  int enumerate(const std::span<const char*>& labels) override;
  std::tuple<const std::uint8_t*, std::size_t> binary(std::size_t length, std::size_t stride)
      override;

  bool optional_begin() override;
  void optional_end() override {}

  int variant_begin(const std::span<const char*>& labels) override;
  void variant_end() override {}

  void object_begin(std::size_t) override {}
  void object_end(std::size_t) override {}
  void object_next(const char* key) override {}

  void tuple_begin(std::size_t trivial_size = 0) override {}
  void tuple_end(std::size_t trivial_size = 0) override {}
  void tuple_next() override {}

  void list_begin(bool is_trivial = false) override;
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
