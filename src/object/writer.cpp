#include "datapack/encode/base64.hpp"
#include "datapack/object.hpp"
#include <cstring>

namespace datapack {

ObjectWriter::ObjectWriter(Object::Reference object) : object(object), next_stride(0) {}

void ObjectWriter::number(NumberType type, const void* value_in) {
  Object::number_t value;
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
  set_value(value);
}

void ObjectWriter::boolean(bool value) { set_value(value); }

void ObjectWriter::string(const char* value) { set_value(std::string(value)); }

void ObjectWriter::enumerate(int value, const std::span<const char*>& labels) {
  set_value(std::string(labels[value]));
}

void ObjectWriter::binary(const std::span<const std::uint8_t>& data) {
  std::vector<std::uint8_t> vec(data.size());
  std::memcpy(vec.data(), data.data(), data.size());
  set_value(vec);
}

void ObjectWriter::optional_begin(bool has_value) {
  if (!has_value) {
    set_value(Object::null_t());
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

void ObjectWriter::variant_end() { object_end(); }

void ObjectWriter::object_begin() { set_value(Object::map_t()); }

void ObjectWriter::object_end() { nodes.pop(); }

void ObjectWriter::object_next(const char* key) { next_key = key; }

void ObjectWriter::tuple_begin() { set_value(Object::list_t()); }

void ObjectWriter::tuple_end() { nodes.pop(); }

void ObjectWriter::tuple_next() { next_key = ""; }

void ObjectWriter::list_begin() { set_value(Object::list_t()); }

void ObjectWriter::list_end() { nodes.pop(); }

void ObjectWriter::list_next() { next_key = ""; }

void ObjectWriter::set_value(const Object::value_t& value) {
  Object::Iterator next;

  if (nodes.empty()) {
    object = value;
    next = object.iter();
  } else {
    const auto& node = nodes.top();
    if (node->is_map()) {
      next = node->insert(next_key, value);
    } else if (node->is_list()) {
      next = node->push_back(value);
    } else {
      throw std::runtime_error("Shouldn't reach here");
    }
  }

  if (next->is_map() || next->is_list()) {
    nodes.push(next);
  }
}

} // namespace datapack
