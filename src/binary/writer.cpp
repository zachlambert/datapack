#include "datapack/binary.hpp"
#include <assert.h>
#include <cstring>
#include <stdexcept>

namespace dpack {

void BinaryWriter::number(NumberType type, const void* value) {
  switch (type) {
  case NumberType::I32:
    value_number(*(std::int32_t*)value);
    break;
  case NumberType::I64:
    value_number(*(std::int64_t*)value);
    break;
  case NumberType::U32:
    value_number(*(std::uint32_t*)value);
    break;
  case NumberType::U64:
    value_number(*(std::uint64_t*)value);
    break;
  case NumberType::U8:
    value_number(*(std::uint8_t*)value);
    break;
  case NumberType::F32:
    value_number(*(float*)value);
    break;
  case NumberType::F64:
    value_number(*(double*)value);
    break;
  }
}

void BinaryWriter::boolean(bool value) {
  value_bool(value);
}

void BinaryWriter::string(const char* value) {
  std::size_t size = std::strlen(value) + 1;
  if (pos_ + size > buffer.size()) {
    throw std::runtime_error("Writer buffer is too small");
  }
  strncpy((char*)&buffer[pos_], value, size);
  pos_ += size;
}

void BinaryWriter::enumerate(int value, const std::span<const char*>& labels) {
  value_number(value);
}

void BinaryWriter::optional_begin(bool has_value) {
  value_bool(has_value);
}

void BinaryWriter::variant_begin(int value, const std::span<const char*>& labels) {
  value_number(value);
}

void BinaryWriter::binary(const std::span<const std::uint8_t>& data) {
  value_number(std::uint64_t(data.size()));
  if (pos_ + data.size() > buffer.size()) {
    throw std::runtime_error("Writer buffer is too small");
  }
  std::memcpy(&buffer[pos_], data.data(), data.size());
  pos_ += data.size();
}

void BinaryWriter::list_begin(size_t size) {
  value_number(size);
}

// Note: Fine to put implementation in source file here, since all usage of the
// method occurs in the same source file
template <typename T>
void BinaryWriter::value_number(T value) {
  if (pos_ + sizeof(T) > buffer.size()) {
    throw std::runtime_error("Writer buffer is too small");
  }
  *((T*)&buffer[pos_]) = value;
  pos_ += sizeof(T);
}

void BinaryWriter::value_bool(bool value) {
  if (pos_ + 1 > buffer.size()) {
    throw std::runtime_error("Writer buffer is too small");
  }
  buffer[pos_] = (value ? 0x01 : 0x00);
  pos_++;
}

} // namespace dpack
