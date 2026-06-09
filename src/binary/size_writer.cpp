#include "datapack/binary.hpp"
#include <assert.h>
#include <cstring>

namespace datapack {

void BinarySizeWriter::number(NumberType type, const void*) {
  switch (type) {
  case NumberType::I32:
    size_ += sizeof(std::int32_t);
    break;
  case NumberType::I64:
    size_ += sizeof(std::int64_t);
    break;
  case NumberType::U32:
    size_ += sizeof(std::uint32_t);
    break;
  case NumberType::U64:
    size_ += sizeof(std::uint64_t);
    break;
  case NumberType::U8:
    size_ += sizeof(std::uint8_t);
    break;
  case NumberType::F32:
    size_ += sizeof(float);
    break;
  case NumberType::F64:
    size_ += sizeof(double);
    break;
  }
}

void BinarySizeWriter::boolean(bool value) {
  size_ += sizeof(value);
}

void BinarySizeWriter::string(const char* value) {
  size_ += std::strlen(value) + 1;
}

void BinarySizeWriter::enumerate(int value, const std::span<const char*>& labels) {
  size_ += sizeof(value);
}

void BinarySizeWriter::optional_begin(bool has_value) {
  size_ += sizeof(has_value);
}

void BinarySizeWriter::variant_begin(int value, const std::span<const char*>& labels) {
  size_ += sizeof(value);
}

void BinarySizeWriter::binary(const std::span<const std::uint8_t>& data) {
  size_ += sizeof(data.size());
  size_ += data.size();
}

void BinarySizeWriter::list_begin(size_t size) {
  size_ += sizeof(size);
}

} // namespace datapack
