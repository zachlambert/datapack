#pragma once

#include "datapack/datapack.hpp"
#include <stack>
#include <vector>

namespace datapack {

class BinaryWriter : public Writer {
public:
  BinaryWriter(std::vector<std::uint8_t>& buffer) : buffer(buffer), pos(0) {}

  void number(NumberType type, const void* value) override;
  void boolean(bool value) override;
  void string(const char* value) override;
  void enumerate(int value, const char* label) override;
  void binary(const std::span<const std::uint8_t>& data) override;

  void optional_begin(bool has_value) override;
  void optional_end() override {}

  void variant_begin(int value, const char* label) override;
  void variant_end() override {}

  void object_begin() override {}
  void object_next(const char* key) override {};
  void object_end() override {}

  void list_begin() override;
  void list_next() override;
  void list_end() override;

  void tuple_begin() override {}
  void tuple_next() override {}
  void tuple_end() override {}

  std::span<std::uint8_t> result() const { return std::span(&buffer[0], pos); }

private:
  template <typename T>
  void value_number(T value);
  void value_bool(bool value);

  std::vector<std::uint8_t>& buffer;
  std::size_t pos;
  std::stack<std::uint64_t> list_length_offsets;
};

class BinaryReader : public Reader {
public:
  BinaryReader(const std::span<const std::uint8_t>& buffer) : buffer(buffer), pos(0) {}

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
  void object_next(const char* key) override {}
  void object_end() override {}

  void tuple_begin() override {}
  void tuple_next() override {}
  void tuple_end() override {}

  void list_begin() override;
  bool list_next() override;
  void list_end() override;

private:
  template <typename T>
  void value_number(T& value);
  bool value_bool();

  std::span<const std::uint8_t> buffer;
  std::size_t pos;
  std::stack<int> list_remaining;
};

template <writeable T>
std::vector<std::uint8_t> write_binary(const T& value) {
  std::vector<std::uint8_t> data;
  BinaryWriter(data).value(value);
  return data;
}

template <readable T>
T read_binary(const std::span<const std::uint8_t>& data) {
  T result;
  BinaryReader(data).value(result);
  return result;
}

} // namespace datapack
