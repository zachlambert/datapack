#pragma once

#include "datapack/datapack.hpp"
#include <stdexcept>
#include <vector>

namespace dpack {

class BinarySizeWriter : public Writer {
public:
  BinarySizeWriter() : size_(0) {}

  void number(NumberType type, const void* value) override;
  void boolean(bool value) override;
  void string(const char* value) override;
  void enumerate(int value, const std::span<const char*>& labels) override;
  void binary(const std::span<const std::uint8_t>& data) override;

  void optional_begin(bool has_value) override;
  void optional_end() override {}

  void variant_begin(int value, const std::span<const char*>& labels) override;
  void variant_end() override {}

  void object_begin() override {}
  void object_next(const char* key) override {};
  void object_end() override {}

  void list_begin(size_t size) override;
  void list_next() override {}
  void list_end() override {}

  void tuple_begin() override {}
  void tuple_next() override {}
  void tuple_end() override {}

  size_t size() const {
    return size_;
  }

private:
  std::size_t size_;
};

class BinaryWriter : public Writer {
public:
  BinaryWriter(std::span<std::uint8_t> buffer) : buffer(buffer), pos_(0) {}

  void number(NumberType type, const void* value) override;
  void boolean(bool value) override;
  void string(const char* value) override;
  void enumerate(int value, const std::span<const char*>& labels) override;
  void binary(const std::span<const std::uint8_t>& data) override;

  void optional_begin(bool has_value) override;
  void optional_end() override {}

  void variant_begin(int value, const std::span<const char*>& labels) override;
  void variant_end() override {}

  void object_begin() override {}
  void object_next(const char* key) override {};
  void object_end() override {}

  void list_begin(size_t size) override;
  void list_next() override {}
  void list_end() override {}

  void tuple_begin() override {}
  void tuple_next() override {}
  void tuple_end() override {}

  size_t pos() const {
    return pos_;
  }

private:
  template <typename T>
  void value_number(T value);
  void value_bool(bool value);

  std::span<std::uint8_t> buffer;
  std::size_t pos_;
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

  size_t list_begin() override;
  void list_next() override {}
  void list_end() override {}

private:
  template <typename T>
  void value_number(T& value);
  bool value_bool();

  std::span<const std::uint8_t> buffer;
  std::size_t pos;
};

template <writeable T>
size_t binary_size(const T& value) {
  BinarySizeWriter size_writer;
  size_writer.value(value);
  return size_writer.size();
}

template <writeable T>
std::vector<std::uint8_t> to_binary(const T& value) {
  std::vector<std::uint8_t> buffer(binary_size(value));
  BinaryWriter writer(buffer);
  writer.value(value);
  if (writer.pos() != buffer.size()) {
    throw std::runtime_error("Write size did not match buffer size");
  }
  return buffer;
}

template <writeable T>
void to_binary(const T& value, const std::span<std::uint8_t>& buffer) {
  BinaryWriter writer(buffer);
  writer.value(value);
  if (writer.pos() != buffer.size()) {
    throw std::runtime_error("Write size did not match buffer size");
  }
}

template <readable T>
T from_binary(const std::span<const std::uint8_t>& buffer) {
  T result;
  BinaryReader(buffer).value(result);
  return result;
}

} // namespace dpack
