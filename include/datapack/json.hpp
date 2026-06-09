#pragma once

#include "datapack/object.hpp"

namespace datapack {

class JsonLoadError : public std::runtime_error {
public:
  JsonLoadError(const std::string& message) : std::runtime_error(message) {}
};

Object load_json(const std::string& json);
std::string dump_json(ConstObject object);

Object load_json_file(const std::string& file);
void dump_json_file(ConstObject object, const std::string& file);

template <readable T>
T from_json(const std::string& json) {
  Object object = load_json(json);
  T result;
  ObjectReader(object).value(result);
  return result;
}

template <writeable T>
std::string to_json(const T& value) {
  Object object;
  ObjectWriter(object).value(value);
  return dump_json(object);
}

template <readable T>
T from_json_file(const std::string& file) {
  Object object = load_json_file(file);
  T result;
  ObjectReader(object).value(result);
  return result;
}

template <writeable T>
void to_json_file(const T& value, const std::string& file) {
  Object object;
  ObjectWriter(object).value(value);
  dump_json_file(object, file);
}

} // namespace datapack
