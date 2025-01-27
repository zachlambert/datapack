#pragma once
#ifndef EMBEDDED

#include "datapack/object.hpp"
#include "datapack/schema/schema.hpp"

namespace datapack {

Object binary_to_object(const Schema &schema,
                        const std::vector<std::uint8_t> &bytes);
std::vector<std::uint8_t>
object_to_binary(const Schema &schema, const Object::ConstReference &object);

} // namespace datapack
#endif
