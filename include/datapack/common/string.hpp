#pragma once

#include "datapack/packers.hpp"


namespace datapack {

inline void pack(const std::string& value, Writer& writer) {
    writer.string(value.c_str());
}

inline void pack(std::string& value, Reader& reader) {
    if (auto str = reader.string()) {
        value = str;
    } else {
        value.clear();
    }
}

} // namespace datapack
