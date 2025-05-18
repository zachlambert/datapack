#include "datapack/binary.hpp"
#include <assert.h>
#include <cstring>

namespace datapack {

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

void BinaryWriter::boolean(bool value) { value_bool(value); }

void BinaryWriter::string(const char* value) {
  std::size_t size = std::strlen(value) + 1;
  buffer.resize(pos + size);
  strncpy((char*)&buffer[pos], value, size);
  pos += size;
}

void BinaryWriter::enumerate(int value, const std::span<const char*>& labels) {
  value_number(value);
}

void BinaryWriter::optional_begin(bool has_value) { value_bool(has_value); }

void BinaryWriter::variant_begin(int value, const std::span<const char*>& labels) {
  value_number(value);
}

void BinaryWriter::binary(const std::span<const std::uint8_t>& data) {
  value_number(std::uint64_t(data.size()));
  buffer.resize(pos + data.size());
  std::memcpy(&buffer[pos], data.data(), data.size());
  pos += data.size();
}

void BinaryWriter::list_begin() {
  list_length_offsets.push(pos);
  value_number(std::uint64_t(0));
}

void BinaryWriter::list_next() {
  assert(!list_length_offsets.empty());
  std::uint64_t& length = *((std::uint64_t*)(buffer.data() + list_length_offsets.top()));
  length++;
}

void BinaryWriter::list_end() {
  //
  list_length_offsets.pop();
}

// Note: Fine to put implementation in source file here, since all usage of the
// method occurs in the same source file
template <typename T>
void BinaryWriter::value_number(T value) {
  buffer.resize(pos + sizeof(T));
  *((T*)&buffer[pos]) = value;
  pos += sizeof(T);
}

void BinaryWriter::value_bool(bool value) {
  buffer.resize(pos + 1);
  buffer[pos] = (value ? 0x01 : 0x00);
  pos++;
}

} // namespace datapack
