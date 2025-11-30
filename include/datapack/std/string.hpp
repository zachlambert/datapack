#pragma once

#include "datapack/datapack.hpp"
#include <string>

namespace datapack {

inline void write(const std::string& value, Writer& writer) {
  writer.string(value.c_str());
}

inline void read(std::string& value, Reader& reader) {
  if (auto str = reader.string()) {
    value = str;
  } else {
    value.clear();
  }
}

} // namespace datapack
