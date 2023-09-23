#pragma once

#include "datapack/datapack.hpp"
#include <iostream>

namespace datapack {

class DebugWriter : public Writer {
public:
  DebugWriter(std::ostream& os);

  void number(NumberType type, const void* value) override;
  void boolean(bool value) override;
  void string(const char* value) override;
  void enumerate(int value, const std::span<const char*>& labels) override;
  void binary(const std::span<const std::uint8_t>& data) override;

  void optional_begin(bool has_value) override;
  void optional_end() override;

  void variant_begin(int value, const std::span<const char*>& labels) override;
  void variant_end() override;

  void object_begin() override;
  void object_next(const char* key) override;
  void object_end() override;

  void tuple_begin() override;
  void tuple_next() override;
  void tuple_end() override;

  void list_begin() override;
  void list_next() override;
  void list_end() override;

private:
  void indent();

  std::ostream& os;
  int depth;
};

template <writeable T>
class debug {
public:
  debug(const T& value) : value(value) {}
  std::ostream& operator()(std::ostream& os) const {
    DebugWriter(os).value(value);
    return os;
  }

private:
  const T& value;
};

} // namespace datapack

template <datapack::writeable T>
std::ostream& operator<<(std::ostream& os, datapack::debug<T> manipulator) {
  return manipulator(os);
}
