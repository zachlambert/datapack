#pragma once

#include "datapack/datapack.hpp"
#include <string>

namespace dpack {

inline void write(Writer& writer, const std::string& value) {
  writer.string(value.c_str());
}

inline void read(Reader& reader, std::string& value) {
  if (auto str = reader.string()) {
    value = str;
  } else {
    value.clear();
  }
}

} // namespace dpack
