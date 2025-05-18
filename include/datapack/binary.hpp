#pragma once

#include "datapack/datapack.hpp"

namespace datapack {

class BinaryReader : public Reader {
public:
  BinaryReader(const std::span<const std::uint8_t>& data, bool trivial_as_binary = true) :
      Reader(trivial_as_binary),
      data(data),
      pos(0),
      binary_depth(0),
      binary_start(0),
      trivial_list_remaining(0) {}

  void number(NumberType type, void* value) override;
  bool boolean() override;
  const char* string() override;
  int enumerate(const std::span<const char*>& labels) override;
  std::span<const std::uint8_t> binary() override;

  bool optional_begin() override;
  void optional_end() override {}

  int variant_begin(const std::span<const char*>& labels) override;
  void variant_end() override {}

  void object_begin() override;
  void object_next(const char* key) override {}
  void object_end() override;

  void tuple_begin() override { object_begin(); }
  void tuple_next() override {}
  void tuple_end() override { object_end(); }

  void list_begin() override;
  bool list_next() override;
  void list_end() override;

private:
  template <typename T>
  void value_number(T& value);
  bool value_bool();

  std::span<const std::uint8_t> data;
  std::size_t pos;
  std::size_t binary_depth;
  std::int64_t binary_start;
  int trivial_list_remaining;
};

template <readable T>
T read_binary(const std::span<const std::uint8_t>& data) {
  T result;
  BinaryReader(data).value(result);
  return result;
}

} // namespace datapack
