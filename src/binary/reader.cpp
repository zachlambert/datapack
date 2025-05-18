#include "datapack/format/binary_reader.hpp"
#include <assert.h>
#include <cstring>

namespace datapack {

void BinaryReader::integer(IntType type, void* value) {
  switch (type) {
  case IntType::I32:
    value_number(*(std::int32_t*)value);
    break;
  case IntType::I64:
    value_number(*(std::int64_t*)value);
    break;
  case IntType::U32:
    value_number(*(std::uint32_t*)value);
    break;
  case IntType::U64:
    value_number(*(std::uint64_t*)value);
    break;
  case IntType::U8:
    value_number(*(std::uint8_t*)value);
    break;
  }
}

void BinaryReader::floating(FloatType type, void* value) {
  switch (type) {
  case FloatType::F32:
    value_number(*(float*)value);
    break;
  case FloatType::F64:
    value_number(*(double*)value);
    break;
  }
}

bool BinaryReader::boolean() { return value_bool(); }

const char* BinaryReader::string() {
  std::size_t max_len = data.size() - pos;
  std::size_t len = strnlen((char*)&data[pos], max_len);
  if (len == max_len) {
    invalidate();
    return nullptr;
  }
  const char* result = (char*)&data[pos];
  pos += (len + 1);
  return result;
}

int BinaryReader::enumerate(const std::span<const char*>& labels) {
  int value = -1;
  value_number(value);
  return value;
}

bool BinaryReader::optional_begin() { return value_bool(); }

int BinaryReader::variant_begin(const std::span<const char*>& labels) {
  int value = -1;
  value_number(value);
  return value;
}

std::tuple<const std::uint8_t*, std::size_t> BinaryReader::binary(
    std::size_t length,
    std::size_t stride) {
  if (length == 0) {
    value_number(length);
  }
  std::size_t size = length * stride;
  if (pos + size > data.size()) {
    invalidate();
    return {nullptr, 0};
  }

  const std::uint8_t* output_data = &data[pos];
  pos += size;
  return std::make_tuple(output_data, length);
}

void BinaryReader::object_begin(std::size_t size) {
  if (size == 0) {
    return;
  }
  if (binary_depth == 0) {
    binary_start = pos;
  }
  pad(size);
  binary_depth++;
}

void BinaryReader::object_end(std::size_t size) {
  if (size == 0) {
    return;
  }
  pad(size);
  binary_depth--;
}

void BinaryReader::list_begin(bool is_trivial) {
  if (binary_depth != 0) {
    invalidate();
    return;
  }
  if (!is_trivial) {
    return;
  }
  std::uint64_t length = 0;
  value_number(length);
  trivial_list_remaining = length;

  binary_depth++;
  binary_start = pos;
}

bool BinaryReader::list_next() {
  if (binary_depth > 0) {
    if (trivial_list_remaining == 0) {
      return false;
    }
    trivial_list_remaining--;
    return true;
  }
  return value_bool();
}

void BinaryReader::list_end() {
  if (binary_depth == 0) {
    return;
  }
  binary_depth--;
  assert(binary_depth == 0);
}

void BinaryReader::pad(std::size_t size) {
  if ((pos - binary_start) % size != 0) {
    pos += (size - (pos - binary_start) % size);
  }
}

template <typename T>
void BinaryReader::value_number(T& value) {
  if (binary_depth > 0) {
    pad(sizeof(T));
  }
  if (pos + sizeof(T) > data.size()) {
    invalidate();
    return;
  }

  value = *((T*)&data[pos]);
  pos += sizeof(T);
}

bool BinaryReader::value_bool() {
  if (pos + 1 > data.size()) {
    invalidate();
    return false;
  }
  std::uint8_t value_int = *((bool*)&data[pos]);
  if (value_int >= 2) {
    invalidate();
    return false;
  }
  pos++;
  return value_int;
}

} // namespace datapack
