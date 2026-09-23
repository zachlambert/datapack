#include "datapack/object.hpp"
#include <cstring>

namespace dpack {

ObjectWriter::ObjectWriter(Object object) : node(object.ptr()), at_container_begin(false) {}

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

void ObjectWriter::enumerate(int value, const std::span<const std::string_view>& labels) {
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

void ObjectWriter::container_begin(bool is_list) {
  if (is_list) {
    node->to_list();
  } else {
    node->to_map();
  }
  at_container_begin = true;
}

void ObjectWriter::container_end() {
  if (!at_container_begin) {
    node = node.parent();
  }
  at_container_begin = false;
}

void ObjectWriter::variant_begin(int value, const std::span<const std::string_view>& labels) {
  const std::string label(labels[value]);
  // A variant is written as a map of the chosen label and its value, which isn't an object of
  // any type of its own, so it goes through container_begin rather than object_begin
  container_begin(false);
  object_next("type");
  *node = label;
  std::string value_key = "value_" + label;
  object_next(value_key.c_str());
}

void ObjectWriter::variant_end() {
  container_end();
}

void ObjectWriter::object_begin(std::string_view) {
  container_begin(false);
}

void ObjectWriter::object_end() {
  container_end();
}

void ObjectWriter::object_next(const char* key) {
  if (at_container_begin) {
    node = node->emplace(key).ptr();
  } else {
    node = node.parent()->emplace(key).ptr();
  }
  at_container_begin = false;
  assert(node);
}

void ObjectWriter::tuple_begin() {
  container_begin(true);
}

void ObjectWriter::tuple_end() {
  container_end();
}

void ObjectWriter::tuple_next() {
  if (at_container_begin) {
    node = node->emplace_back().ptr();
  } else {
    node = node.parent()->emplace_back().ptr();
  }
  at_container_begin = false;
  assert(node);
}

void ObjectWriter::list_begin(size_t) {
  container_begin(true);
}

void ObjectWriter::list_end() {
  container_end();
}

void ObjectWriter::list_next() {
  if (at_container_begin) {
    node = node->emplace_back().ptr();
  } else {
    node = node.parent()->emplace_back().ptr();
  }
  at_container_begin = false;
  assert(node);
}

} // namespace dpack
