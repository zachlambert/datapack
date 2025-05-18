#include "datapack/encode/base64.hpp"
#include "datapack/object.hpp"

namespace datapack {

ObjectReader::ObjectReader(Object::ConstReference object) :
    node(object.iter()), list_start(false), next_variant_label(nullptr) {}

void ObjectReader::number(NumberType type, void* value_out) {
  Object::number_t value;
  if (auto x = node->number_if()) {
    value = *x;
  } else {
    invalidate();
    return;
  }
  switch (type) {
  case NumberType::I32:
    *(std::int32_t*)value_out = value;
    break;
  case NumberType::I64:
    *(std::int64_t*)value_out = value;
    break;
  case NumberType::U32:
    *(std::uint32_t*)value_out = value;
    break;
  case NumberType::U64:
    *(std::uint64_t*)value_out = value;
    break;
  case NumberType::U8:
    *(std::uint8_t*)value_out = value;
    break;
  case NumberType::F32:
    *(float*)value_out = value;
    break;
  case NumberType::F64:
    *(double*)value_out = value;
    break;
  }
}

bool ObjectReader::boolean() {
  if (auto x = node->boolean_if()) {
    return *x;
  }
  invalidate();
  return false;
}

const char* ObjectReader::string() {
  if (auto x = node->string_if()) {
    return x->c_str();
  }
  invalidate();
  return nullptr;
}

int ObjectReader::enumerate(const std::span<const char*>& labels) {
  auto x = node->string_if();
  if (!x) {
    invalidate();
    return 0;
  }
  for (int i = 0; i < labels.size(); i++) {
    if ((*x) == labels[i]) {
      return i;
    }
  }
  invalidate();
  return 0;
}

std::span<const std::uint8_t> ObjectReader::binary() {
  const std::uint8_t* data = nullptr;
  std::size_t size = 0;

  if (auto x = node->binary_if()) {
    return *x;
  } else if (auto x = node->string_if()) {
    data_temp = base64_decode(*x);
    return data_temp;
  } else {
    invalidate();
    return std::span<const std::uint8_t>((const std::uint8_t*)nullptr, 0);
  }
}

bool ObjectReader::optional_begin() {
  if (node->is_null()) {
    return false;
  }
  return true;
}

void ObjectReader::optional_end() {
  // Do nothing
}

int ObjectReader::variant_begin(const std::span<const char*>& labels) {
  object_begin();
  object_next("type");
  if (auto x = node->string_if()) {
    for (int i = 0; i < labels.size(); i++) {
      if (labels[i] == *x) {
        std::string value_key = "value_" + std::string(labels[i]);
        object_next(value_key.c_str());
        return i;
      }
    }
  }
  invalidate();
  return 0;
}

void ObjectReader::variant_end() { object_end(); }

void ObjectReader::object_begin() {
  nodes.push(node);
  node = node.child();
}

void ObjectReader::object_end() {
  node = nodes.top();
  nodes.pop();
}

void ObjectReader::object_next(const char* key) {
  auto parent = node.parent();
  if (!parent) {
    invalidate();
    return;
  }
  if (!parent->is_map()) {
    invalidate();
    return;
  }
  auto next = parent->find(std::string(key));
  if (!next) {
    invalidate();
    return;
  }
  node = next;
}

void ObjectReader::tuple_begin() {
  if (!node->is_list()) {
    invalidate();
    return;
  }
  list_start = true;
  nodes.push(node);
  node = node.child();
}

void ObjectReader::tuple_next() {
  if (!list_start) {
    node = node.next();
  }
  list_start = false;
  if (!node) {
    invalidate();
    return;
  }
}

void ObjectReader::tuple_end() {
  list_start = false;
  node = nodes.top();
  nodes.pop();
}

void ObjectReader::list_begin() {
  if (!node->is_list()) {
    invalidate();
    return;
  }
  list_start = true;
  nodes.push(node);
  node = node.child();
}

bool ObjectReader::list_next() {
  if (!list_start) {
    node = node.next();
  }
  list_start = false;
  return bool(node);
}

void ObjectReader::list_end() {
  list_start = false;
  node = nodes.top();
  nodes.pop();
}

} // namespace datapack
