#pragma once

#include "datapack/reader.hpp"
#include <vector>

namespace datapack {

class BinaryReader : public Reader {
public:
  BinaryReader(const std::span<const std::uint8_t> &data,
               bool trivial_as_binary = true)
      : Reader(trivial_as_binary), data(data), pos(0), binary_depth(0),
        binary_start(0), trivial_list_remaining(0) {}

  void integer(IntType type, void *value) override;
  void floating(FloatType type, void *value) override;
  bool boolean() override;
  const char *string() override;
  int enumerate(const std::span<const char *> &labels) override;
  std::tuple<const std::uint8_t *, std::size_t>
  binary(std::size_t length, std::size_t stride) override;

  bool optional_begin() override;
  void optional_end() override {}

  int variant_begin(const std::span<const char *> &labels) override;
  void variant_end() override {}

  void object_begin(std::size_t size) override;
  void object_end(std::size_t size) override;
  void object_next(const char *key) override {}

  void tuple_begin(std::size_t size) override { object_begin(size); }
  void tuple_end(std::size_t size) override { object_end(size); }
  void tuple_next() override {}

  void list_begin(bool is_trivial) override;
  bool list_next() override;
  void list_end() override;

private:
  void pad(std::size_t size);
  template <typename T> void value_number(T &value);
  bool value_bool();

  std::span<const std::uint8_t> data;
  std::size_t pos;
  std::size_t binary_depth;
  std::int64_t binary_start;
  int trivial_list_remaining;
};

template <readable T> T read_binary(const std::span<const std::uint8_t> &data) {
  T result;
  BinaryReader(data).value(result);
  return result;
}

} // namespace datapack
