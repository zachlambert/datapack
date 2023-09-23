#include "datapack/schema/token.hpp"
#include "datapack/std/string.hpp"
#include "datapack/std/vector.hpp"

namespace datapack {

DATAPACK_LABELLED_ENUM_DEF(NumberType) = {"i32", "i64", "u32", "u64", "u8", "f32", "f64"};

DATAPACK_IMPL(token::Enumerate, value, packer) {
  packer.object_begin();
  packer.value("labels", value.labels);
  packer.object_end();
}

DATAPACK_IMPL(token::VariantBegin, value, packer) {
  packer.object_begin();
  packer.value("labels", value.labels);
  packer.object_end();
}

DATAPACK_IMPL(token::VariantNext, value, packer) {
  packer.object_begin();
  packer.value("index", value.index);
  packer.object_end();
}

DATAPACK_IMPL(token::Binary, value, packer) {
  packer.object_begin();
  packer.value("length", value.stride);
  packer.value("stride", value.length);
  packer.object_end();
}

DATAPACK_IMPL(token::ObjectNext, value, packer) {
  packer.object_begin();
  packer.value("key", value.key);
  packer.object_end();
}

DATAPACK_LABELLED_VARIANT_DEF(Token) = {
    "number",
    "boolean",
    "string",
    "enumerate",
    "binary",
    "optional",
    "variant_begin",
    "variant_next",
    "variant_end",
    "object_begin",
    "object_next",
    "object_end",
    "tuple_begin",
    "tuple_next",
    "tuple_end",
    "list"};

bool operator==(const Token& lhs, const Token& rhs) {
  if (lhs.index() != rhs.index())
    return false;

  if (auto lhs_value = std::get_if<token::Enumerate>(&lhs)) {
    auto rhs_value = std::get_if<token::Enumerate>(&rhs);
    return lhs_value->labels == rhs_value->labels;
  }
  if (auto lhs_value = std::get_if<token::VariantBegin>(&lhs)) {
    auto rhs_value = std::get_if<token::VariantBegin>(&rhs);
    return lhs_value->labels == rhs_value->labels;
  }
  if (auto lhs_value = std::get_if<token::VariantNext>(&lhs)) {
    auto rhs_value = std::get_if<token::VariantNext>(&rhs);
    return lhs_value->index == rhs_value->index;
  }
  if (auto lhs_value = std::get_if<token::ObjectNext>(&lhs)) {
    auto rhs_value = std::get_if<token::ObjectNext>(&rhs);
    return lhs_value->key == rhs_value->key;
  }

  return true;
}

} // namespace datapack
