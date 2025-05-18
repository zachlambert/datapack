#pragma once
#ifndef EMBEDDED

#include "datapack/object.hpp"

namespace datapack {

class JsonLoadError : public std::runtime_error {
public:
  JsonLoadError(const std::string& message) : std::runtime_error(message) {}
};

Object load_json(const std::string& json);
std::string dump_json(const Object::ConstReference& object);

template <readable T>
T read_json(const std::string& json) {
  Object object = load_json(json);
  T result;
  ObjectReader(object).value(result);
  return result;
}

template <writeable T>
std::string write_json(const T& value) {
  Object object;
  ObjectWriter(object).value(value);
  return dump_json(object);
}

} // namespace datapack
#endif
