#include "datapack/encode/base64.hpp"
#include "datapack/object.hpp"
#include <cstring>

namespace datapack {

ObjectWriter::ObjectWriter(Object object) : node(object.ptr()), container_begin(false) {}

void ObjectWriter::number(NumberType type, const void* value_in) {
  object::number_t value;
  switch (type) {
  case NumberType::I32:
    value = *(std::int32_t*)value_in;
    break;
  case NumberType::I64:
    value = *(std::int64_t*)value_in;
    break;
  case NumberType::U32:
    value = *(std::uint32_t*)value_in;
    break;
  case NumberType::U64:
    value = *(std::uint64_t*)value_in;
    break;
  case NumberType::U8:
    value = *(std::uint8_t*)value_in;
    break;
  case NumberType::F32:
    value = *(float*)value_in;
    break;
  case NumberType::F64:
    value = *(double*)value_in;
    break;
  }
  *node = value;
}

void ObjectWriter::boolean(bool value) {
  *node = value;
}

void ObjectWriter::string(const char* value) {
  *node = value;
}

void ObjectWriter::enumerate(int value, const std::span<const char*>& labels) {
  *node = std::string(labels[value]);
}

void ObjectWriter::binary(const std::span<const std::uint8_t>& data) {
  std::vector<std::uint8_t> bytes(data.size());
  std::memcpy(bytes.data(), data.data(), data.size());
  *node = bytes;
}

void ObjectWriter::optional_begin(bool has_value) {
  if (!has_value) {
    node->to_null();
  }
}

void ObjectWriter::optional_end() {
  // Do nothing
}

void ObjectWriter::variant_begin(int value, const std::span<const char*>& labels) {
  object_begin();
  object_next("type");
  string(labels[value]);
  std::string value_key = "value_" + std::string(labels[value]);
  object_next(value_key.c_str());
}

void ObjectWriter::variant_end() {
  object_end();
}

void ObjectWriter::object_begin() {
  node->to_map();
  container_begin = true;
}

void ObjectWriter::object_end() {
  if (!container_begin) {
    node = node.parent();
  }
  container_begin = false;
}

void ObjectWriter::object_next(const char* key) {
  if (container_begin) {
    node = node->emplace(key).ptr();
  } else {
    node = node.parent()->emplace(key).ptr();
  }
  container_begin = false;
  assert(node);
}

void ObjectWriter::tuple_begin() {
  node->to_list();
  container_begin = true;
}

void ObjectWriter::tuple_end() {
  if (!container_begin) {
    node = node.parent();
  }
  container_begin = false;
}

void ObjectWriter::tuple_next() {
  if (container_begin) {
    node = node->emplace_back().ptr();
  } else {
    node = node.parent()->emplace_back().ptr();
  }
  container_begin = false;
  assert(node);
}

void ObjectWriter::list_begin() {
  node->to_list();
  container_begin = true;
}

void ObjectWriter::list_end() {
  if (!container_begin) {
    node = node.parent();
  }
  container_begin = false;
}

void ObjectWriter::list_next() {
  if (container_begin) {
    node = node->emplace_back().ptr();
  } else {
    node = node.parent()->emplace_back().ptr();
  }
  container_begin = false;
  assert(node);
}

} // namespace datapack
