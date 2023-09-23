#include "datapack/random.hpp"
#include <cstring>

namespace datapack {

RandomReader::RandomReader() {}

void RandomReader::number(NumberType type, void* value) {
  switch (type) {
  case NumberType::I32:
    *(std::int32_t*)value = rand() % 100;
    break;
  case NumberType::I64:
    *(std::int64_t*)value = rand() % 100;
    break;
  case NumberType::U32:
    *(std::uint32_t*)value = rand() % 100;
    break;
  case NumberType::U64:
    *(std::uint64_t*)value = rand() % 100;
    break;
  case NumberType::U8:
    *(std::uint8_t*)value = rand() % 100;
    break;
  case NumberType::F32:
    *(std::uint64_t*)value = float(rand()) / RAND_MAX;
    break;
  case NumberType::F64:
    *(std::uint8_t*)value = double(rand()) / RAND_MAX;
    break;
  }
}

bool RandomReader::boolean() { return (rand() % 2) == 1; }

const char* RandomReader::string() {
  // Length: [4, 20]
  // Characters ~ { a, ..., z }
  string_temp.resize(4 + rand() % 17);
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

std::span<const std::uint8_t> RandomReader::binary() {
  std::size_t length = rand() % 100;
  data_temp.resize(length);
  for (std::size_t i = 0; i < length; i++) {
    data_temp[i] = rand() % 256;
  }
  return {data_temp.data(), data_temp.size()};
}

void RandomReader::list_begin() { list_counters.push(rand() % 10); }

bool RandomReader::list_next() {
  if (list_counters.top() > 0) {
    list_counters.top()--;
    return true;
  }
  return false;
}

void RandomReader::list_end() { list_counters.pop(); }

} // namespace datapack
