#include "datapack/encode/base64.hpp"
#include "datapack/object.hpp"

namespace dpack {

ObjectReader::ObjectReader(ConstObject object) : node(object.ptr()), at_container_begin(false) {}

void ObjectReader::number(NumberType type, void* value_out) {
  object::number_t value;
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

int ObjectReader::enumerate(const std::span<const std::string_view>& labels) {
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

bool ObjectReader::container_begin(bool is_list) {
  if (is_list ? !node->is_list() : !node->is_map()) {
    invalidate();
    return false;
  }
  at_container_begin = true;
  return true;
}

void ObjectReader::container_end() {
  if (!at_container_begin) {
    node = node.parent();
  }
  at_container_begin = false;
}

int ObjectReader::variant_begin(const std::span<const std::string_view>& labels) {
  // A variant is read from a map of the chosen label and its value, which isn't an object of
  // any type of its own, so it goes through container_begin rather than object_begin
  if (!container_begin(false)) {
    return 0;
  }
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

void ObjectReader::variant_end() {
  container_end();
}

void ObjectReader::object_begin(std::string_view) {
  container_begin(false);
}

void ObjectReader::object_end() {
  container_end();
}

void ObjectReader::object_next(const char* key) {
  // Before the first key, node is still the map itself. Afterwards it is the previous value, so
  // the map has to be reached back through its parent
  auto parent = at_container_begin ? node : node.parent();
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
  at_container_begin = false;
  node = next;
}

void ObjectReader::tuple_begin() {
  container_begin(true);
}

void ObjectReader::tuple_next() {
  if (at_container_begin) {
    auto child = node.child();
    if (!child) {
      invalidate();
      return;
    }
    at_container_begin = false;
    node = child;
    return;
  }

  auto next = node.next();
  if (!next) {
    invalidate();
    return;
  }
  node = next;
}

void ObjectReader::tuple_end() {
  container_end();
}

size_t ObjectReader::list_begin() {
  if (!container_begin(true)) {
    return 0;
  }
  return node->size();
}

void ObjectReader::list_next() {
  if (at_container_begin) {
    auto child = node.child();
    if (!child) {
      throw std::runtime_error("At the end of the list, no more items");
    }
    node = child;
    at_container_begin = false;
    return;
  }

  auto next = node.next();
  if (!next) {
    throw std::runtime_error("At the end of the list, no more items");
  }
  node = next;
}

void ObjectReader::list_end() {
  container_end();
}

} // namespace dpack
