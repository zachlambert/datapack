#include "datapack/encode/base64.hpp"
#include "datapack/object.hpp"

namespace datapack {

ObjectReader::ObjectReader(Object::ConstReference object) :
    node(object.iter()), list_start(false), next_variant_label(nullptr) {}

void ObjectReader::integer(IntType type, void* value) {
  std::int64_t integer_value;
  if (auto x = node->integer_if()) {
    integer_value = *x;
  } else {
    invalidate();
    return;
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

void ObjectReader::floating(FloatType type, void* value) {
  double floating_value;
  if (auto x = node->floating_if()) {
    floating_value = *x;
  } else if (auto x = node->integer_if()) {
    floating_value = *x;
  } else {
    invalidate();
    return;
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

std::tuple<const std::uint8_t*, std::size_t> ObjectReader::binary(
    std::size_t length,
    std::size_t stride) {
  const std::uint8_t* data = nullptr;
  std::size_t size = 0;

  if (auto x = node->binary_if()) {
    if (x->size() % stride != 0) {
      invalidate();
      return {nullptr, 0};
    }
    if (length != 0 && length * stride != x->size()) {
      invalidate();
      return {nullptr, 0};
    }
    data = x->data();
    size = x->size();
  } else if (auto x = node->string_if()) {
    data_temp = base64_decode(*x);
    data = data_temp.data();
    size = data_temp.size();
  } else {
    invalidate();
    return {nullptr, 0};
  }

  if (size % stride != 0) {
    invalidate();
    return {nullptr, 0};
  } else if (length == 0) {
    length = size / stride;
  } else if (length * stride != size) {
    invalidate();
    return {nullptr, 0};
  }

  return std::make_tuple(data, length);
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
  object_begin(0);
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

void ObjectReader::variant_end() { object_end(0); }

void ObjectReader::object_begin(std::size_t size) {
  nodes.push(node);
  node = node.child();
}

void ObjectReader::object_end(std::size_t size) {
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

void ObjectReader::tuple_begin(std::size_t size) {
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

void ObjectReader::tuple_end(std::size_t size) {
  list_start = false;
  node = nodes.top();
  nodes.pop();
}

void ObjectReader::list_begin(bool is_trivial) {
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
