#include "datapack/binary.hpp"
#include <assert.h>
#include <cstring>

namespace datapack {

void BinaryReader::number(NumberType type, void* value) {
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

bool BinaryReader::boolean() {
  return value_bool();
}

const char* BinaryReader::string() {
  std::size_t max_len = buffer.size() - pos;
  std::size_t len = strnlen((char*)&buffer[pos], max_len);
  if (len == max_len) {
    invalidate();
    return nullptr;
  }
  const char* result = (char*)&buffer[pos];
  pos += (len + 1);
  return result;
}

int BinaryReader::enumerate(const std::span<const char*>& labels) {
  int value = -1;
  value_number(value);
  return value;
}

bool BinaryReader::optional_begin() {
  return value_bool();
}

int BinaryReader::variant_begin(const std::span<const char*>& labels) {
  int value = -1;
  value_number(value);
  return value;
}

std::span<const std::uint8_t> BinaryReader::binary() {
  std::uint64_t length;
  value_number(length);
  auto result = std::span(buffer.data() + pos, length);
  pos += length;
  return result;
}

void BinaryReader::list_begin() {
  std::uint64_t length;
  value_number(length);
  list_remaining.push(length);
}

bool BinaryReader::list_next() {
  assert(!list_remaining.empty());
  if (list_remaining.top() > 0) {
    list_remaining.top()--;
    return true;
  }
  return false;
}

void BinaryReader::list_end() {
  assert(!list_remaining.empty());
  assert(list_remaining.top() == 0);
  list_remaining.pop();
}

template <typename T>
void BinaryReader::value_number(T& value) {
  if (pos + sizeof(T) > buffer.size()) {
    invalidate();
    return;
  }

  value = *((T*)&buffer[pos]);
  pos += sizeof(T);
}

bool BinaryReader::value_bool() {
  if (pos + 1 > buffer.size()) {
    invalidate();
    return false;
  }
  std::uint8_t value_int = *((bool*)&buffer[pos]);
  if (value_int >= 2) {
    invalidate();
    return false;
  }
  pos++;
  return value_int;
}

} // namespace datapack
