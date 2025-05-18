#include "datapack/util/random.hpp"
#include <cstring>

namespace datapack {

RandomReader::RandomReader() {}

void RandomReader::integer(IntType type, void* value) {
  std::int64_t integer_value;
  if (auto c = constraint<RangeConstraint>()) {
    integer_value = std::int64_t(c->lower) + rand() % std::int64_t(c->upper - c->lower);
  } else if (type == IntType::U8) {
    integer_value = rand() % 256;
  } else if (type == IntType::I32 || type == IntType::I64) {
    integer_value = -100 + rand() % 200;
  } else {
    integer_value = rand() % 100;
  }
  switch (type) {
  case IntType::I32:
    *(std::int32_t*)value = integer_value;
    break;
  case IntType::I64:
    *(std::int64_t*)value = integer_value;
    break;
  case IntType::U32:
    *(std::uint32_t*)value = integer_value;
    break;
  case IntType::U64:
    *(std::uint64_t*)value = integer_value;
    break;
  case IntType::U8:
    *(std::uint8_t*)value = integer_value;
    break;
  }
}

void RandomReader::floating(FloatType type, void* value) {
  double floating_value;
  if (auto c = constraint<RangeConstraint>()) {
    floating_value = c->lower + (c->upper - c->lower) * rand() / RAND_MAX;
  } else {
    floating_value = double(rand()) / RAND_MAX;
  }
  switch (type) {
  case FloatType::F32:
    *(float*)value = floating_value;
    break;
  case FloatType::F64:
    *(double*)value = floating_value;
    break;
  }
}

bool RandomReader::boolean() { return (rand() % 2) == 1; }

const char* RandomReader::string() {
  // Length: [4, 20]
  // Characters ~ { a, ..., z }
  if (auto c = constraint<LengthConstraint>()) {
    string_temp.resize(c->length);
  } else {
    string_temp.resize(4 + rand() % 17);
  }
  for (auto& c : string_temp) {
    c = 'a' + rand() % 26;
  }
  return string_temp.c_str();
}

int RandomReader::enumerate(const std::span<const char*>& labels) { return rand() % labels.size(); }

bool RandomReader::optional_begin() { return rand() % 2 == 1; }

int RandomReader::variant_begin(const std::span<const char*>& labels) {
  return rand() % labels.size();
}

std::tuple<const std::uint8_t*, std::size_t> RandomReader::binary(
    std::size_t length,
    std::size_t stride) {
  if (length == 0) {
    length = rand() % 100;
  }
  std::size_t size = length * stride;
  data_temp.resize(size);
  for (std::size_t i = 0; i < size; i++) {
    data_temp[i] = rand() % 256;
  }
  return {data_temp.data(), data_temp.size()};
}

void RandomReader::list_begin(bool is_trivial) { list_counters.push(rand() % 10); }

bool RandomReader::list_next() {
  if (list_counters.top() > 0) {
    list_counters.top()--;
    return true;
  }
  return false;
}

void RandomReader::list_end() { list_counters.pop(); }

} // namespace datapack
