#pragma once

#include <sstream>
#include <stack>
#include "datapack/writer.hpp"
#include "datapack/reader.hpp"
#include "datapack/object.hpp"
#include "datapack/util/object_reader.hpp"
#include "datapack/util/object_writer.hpp"

namespace datapack {

Object load_json(const std::string& json);
std::string dump_json(ConstObject object);

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

} // namespace datpack
